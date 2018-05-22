#include "entities.hpp"
#include "timer.hpp"

#include <experimental/optional>
#include <future>
#include <iostream>
#include <sstream>
#include <string>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

using std::cout;
using std::endl;
using std::make_unique;
using std::move;
using std::string;
using std::to_string;
using std::atomic_size_t;
using std::atomic_bool;
using std::mutex;
using std::out_of_range;
using std::async;
using std::launch;
using std::ref;
using std::cref;
using std::future;
using std::vector;
using std::experimental::optional;
using std::experimental::nullopt;
using nlohmann::json;

auto constexpr maxThreads = size_t{10};

struct RequestPage {
    const size_t _page;

    constexpr RequestPage(size_t page)
        : _page{page == 0 ? throw out_of_range{"page can't be zero"} : page}
    {
    }

    auto toString() const -> string { return to_string(_page); }
};

auto getPage(RequestPage const& page) -> optional<string>
{
    auto myUrl = make_unique<curlpp::options::Url>(
        "https://games.crossfit.com/competitions/api/v1/competitions/open/2018/"
        "leaderboards?page="
        + page.toString());
    curlpp::Easy myRequest;
    myRequest.setOpt(move(myUrl));
    std::ostringstream os;
    curlpp::options::WriteStream ws(&os);
    myRequest.setOpt(ws);

    try {
        myRequest.perform();
    } catch (curlpp::RuntimeError& e) {
        cout << e.what() << endl;
        return nullopt;
    } catch (curlpp::LogicError& e) {
        cout << e.what() << endl;
    }

    return os.str();
}

auto getAnotherPage(atomic_size_t& current, atomic_size_t const& total) -> void
{
    while (current <= total) {
        auto const response = getPage(current++);

        try {
            auto const j3 = json::parse(*response);
            const auto page = j3.get<entities::Page>();
            cout << page.pagination.currentPage << "/" << total << '\n';
        } catch (...) {
            cout << "Parse error\n";
        }
    }
}

auto main() -> int
{
    Timer tmr;
    tmr.reset();
    auto currentPage = atomic_size_t{1};
    auto totalPages = atomic_size_t{
        json::parse(*getPage(1)).get<entities::Page>().pagination.totalPages};
    auto futs = vector<future<void>>{};

    for (auto i = size_t{0}; i < maxThreads; ++i) {
        auto fut = async(launch::async, getAnotherPage, ref(currentPage),
                         cref(totalPages));
        futs.push_back(move(fut));
    }

    for (auto const& f : futs) {
        f.wait();
    }

    cout << tmr.elapsed() << '\n';
    return 0;
}
