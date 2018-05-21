#include "entities.hpp"
#include "timer.hpp"

#include <experimental/optional>
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
using std::out_of_range;
using std::experimental::optional;
using nlohmann::json;

struct RequestPage {
    const size_t _page;

    constexpr RequestPage(size_t page)
        : _page{page == 0 ? throw out_of_range{"page can't be zero"} : page}
    {
    }

    string toString() const { return to_string(_page); }
};

optional<string> getPage(RequestPage const& page)
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
    } catch (curlpp::LogicError& e) {
        cout << e.what() << endl;
    }

    return os.str();
}

int main()
{
    Timer tmr;
    tmr.reset();
    auto totalPages = size_t{1};
    auto currentPage = size_t{1};

    while (currentPage <= totalPages) {
        const auto j3 = json::parse(*getPage(1));
        const auto page = j3.get<entities::Page>();
        //cout << j3.dump();
        totalPages = page.pagination.totalPages;
        ++currentPage;
        cout << currentPage << "/" << totalPages << '\n';
    }

    cout << tmr.elapsed() << '\n';

    return 0;
}
