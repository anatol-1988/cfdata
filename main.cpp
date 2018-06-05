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

#include <sys/stat.h>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

using nlohmann::json;
using nlohmann::detail::parse_error;
using std::async;
using std::atomic_bool;
using std::atomic_size_t;
using std::boolalpha;
using std::condition_variable;
using std::cout;
using std::cref;
using std::endl;
using std::future;
using std::ifstream;
using std::istreambuf_iterator;
using std::launch;
using std::make_unique;
using std::move;
using std::mutex;
using std::ofstream;
using std::ostream;
using std::out_of_range;
using std::quoted;
using std::ref;
using std::runtime_error;
using std::string;
using std::to_string;
using std::unique_lock;
using std::vector;
using std::experimental::nullopt;
using std::experimental::optional;

auto constexpr maxThreads = size_t{10};

struct RequestPage {
    const size_t _page;

    constexpr RequestPage(size_t page)
        : _page{page == 0 ? throw out_of_range{"page can't be zero"} : page}
    {
    }

    auto toString() const -> string { return to_string(_page); }
};

auto mkdir(string const &name) -> void
{
    mkdir(name.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

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

    auto const str = os.str();

    try {
        auto const j3 = json::parse(str);
        auto fout = ofstream{"json/" + page.toString() + ".json"};
        fout << j3.dump();
        return str;
    } catch (...) {
        cout << "Error during parsing " << page.toString() << " page\n";
        cout << str << "\n";
        return nullopt;
    }
}

auto fileExist(string const &name) -> bool
{
    auto const f = ifstream{name.c_str()};
    return f.good();
}

auto fetchPage(RequestPage const &page) -> optional<string>
{
    auto const filename = "json/" + page.toString() + ".json";

    if (fileExist(filename)) {
        cout << "Reading " << filename << " file\n";
        auto f = ifstream{filename.c_str()};
        auto const str
            = string{istreambuf_iterator<char>{f}, istreambuf_iterator<char>{}};
        return str;
    } else {
        cout << "Requesting " << page.toString() << " page\n";
        return requestPage(page);
    }
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
        auto const response = fetchPage(current++);

        try {
            auto const j3 = json::parse(*response);
            const auto page = j3.get<entities::Page>();
            queue.push_back(page);
        } catch (parse_error const &error) {
            cout << error.what() << "\n";
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

auto getScore(string const &str) -> string
{
    if (str.empty())
        return "null,null";

    auto copy = str;

    if (auto const pos = copy.rfind(" - s"); pos != string::npos)
        copy.resize(pos);

    if (auto const pos = copy.rfind(" lb"); pos != string::npos) {
        copy.resize(pos);
        return copy + ",null";
    }

    if (auto const pos = copy.rfind(" reps"); pos != string::npos) {
        copy.resize(pos);
        return copy + ",null";
    }

    return "null," + copy;
}

auto operator<<(ostream &out, entities::Score const &score) -> ostream &
{
    auto const result = getScore(score.scoreDisplay);
    out << (result.empty() or result == " " ? "null" : result) << ","
        << boolalpha << score.scaled << ",";
    return out;
}

auto printOnePage(ostream &out, Page const &page) -> void
{
    for (auto const &entrant : page.leaderboardRows) {
        out << page.pagination.currentPage << ","
            << quoted(entrant.entrant.competitorId) << ","
            << quoted(entrant.entrant.competitorName) << ","
            << quoted(entrant.entrant.divisionId) << ",";

        auto i = size_t{1};

        for (auto const &score : entrant.scores) {
            if (score.ordinal == i)
                out << score;
            else
                out << "null,null";

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
    mkdir("json");
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

    auto out = ofstream{"cfopen2018.csv"};
    auto fut = async(launch::async, printPages, ref(out));

    for (auto const &f : futs)
        f.wait();

    fut.wait();
    cout << tmr.elapsed() << '\n';
    return 0;
}
