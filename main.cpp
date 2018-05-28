#include "entities.hpp"
#include "queue.hpp"
#include "timer.hpp"

#include <condition_variable>
#include <experimental/optional>
#include <fstream>
#include <future>
#include <iostream>
#include <sstream>
#include <string>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

using std::cout;
using std::endl;
using std::ostream;
using std::ofstream;
using std::make_unique;
using std::move;
using std::string;
using std::quoted;
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
using std::condition_variable;
using std::unique_lock;
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

auto requestPage(RequestPage const &page) -> optional<string>
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
    } catch (curlpp::RuntimeError &e) {
        cout << e.what() << "\n";
        return nullopt;
    } catch (curlpp::LogicError &e) {
        cout << e.what() << "\n";
        return nullopt;
    }

    return os.str();
}

static auto m = mutex{};
static auto cv = condition_variable{};
static auto newPage = false;
static auto queue = Queue{};
static auto printPage = size_t{1};
static auto totalPages = atomic_size_t{0};

auto getAnotherPage(atomic_size_t &current, atomic_size_t const &total) -> void
{
    while (current <= total) {
        auto const response = requestPage(current++);

        try {
            auto const j3 = json::parse(*response);
            auto out = ofstream{to_string(current) + ".json"};
            out << j3.dump();
            // const auto page = j3.get<entities::Page>();
            // cout << page.pagination.currentPage << "/" << total << '\n';
            // queue.push_back(page);
        } catch (...) {
            cout << "Parse error\n";
        }

        newPage = true;
        cv.notify_one();
    }
}

auto operator<<(ostream &out, optional<size_t> const &opt) -> ostream &
{
    if (opt)
        out << *opt;
    else
        out << "";

    return out;
}

auto operator<<(ostream &out, entities::Score const &score) -> ostream &
{
    out << quoted(score.scoreDisplay) << ", " << score.scaled << ", "
        << score.time;
    return out;
}

auto printOnePage(ostream &out, Page const &page) -> void
{
    for (auto const &entrant : page.leaderboardRows) {
        out << page.pagination.currentPage << ", "
            << quoted(entrant.entrant.competitorId) << ", "
            << quoted(entrant.entrant.competitorName) << ", "
            << quoted(entrant.entrant.divisionId) << ", ";

        auto i = size_t{1};

        for (auto const &score : entrant.scores) {
            if (score.ordinal == i)
                out << ", " << score;
            else
                out << ", , , , ,";

            ++i;
        }

        out << "\n";
    }
}

auto printPages(ostream &out) -> void
{
    while (printPage <= totalPages) {
        unique_lock<mutex> lk{m};
        cv.wait(lk, [] { return newPage; });
        auto page = queue.get(printPage);

        while (page) {
            printOnePage(out, *page);
            ++printPage;
            page = queue.get(printPage);
        }
    }
}

auto main() -> int
{
    Timer tmr;
    tmr.reset();
    auto currentPage = atomic_size_t{1};
    totalPages = json::parse(*requestPage(1))
                     .get<entities::Page>()
                     .pagination.totalPages;
    auto futs = vector<future<void>>{};

    for (auto i = size_t{0}; i < maxThreads; ++i) {
        auto fut = async(launch::async, getAnotherPage, ref(currentPage),
                         cref(totalPages));
        futs.push_back(move(fut));
    }

    auto out = ofstream{"out.txt"};
    auto fut = async(launch::async, printPages, ref(out));

    for (auto const &f : futs)
        f.wait();

    fut.wait();
    cout << tmr.elapsed() << '\n';
    return 0;
}
