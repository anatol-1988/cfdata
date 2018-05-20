#include <experimental/optional>
#include <iostream>
#include <sstream>
#include <string>

#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>

#include <nlohmann/json.hpp>

using std::cout;
using std::endl;
using std::make_unique;
using std::move;
using std::string;
using std::vector;
using std::experimental::optional;
using std::experimental::nullopt;
using nlohmann::json;

struct Pagination {
    size_t currentPage;
    size_t totalPages;
    size_t totalCompetitors;
};

void from_json(const json& j, Pagination& p) {
    p.currentPage = j.at("currentPage").get<size_t>();
    p.totalPages = j.at("totalPages").get<size_t>();
    p.totalCompetitors = j.at("totalCompetitors").get<size_t>();
}

struct Entrant {
    string competitorId;
    string competitorName;
    string status;
    string postCompStatus;
    string gender;
    string age;
    string height;
    string weight;
};

void from_json(const json& j, Entrant& e)
{
    e.competitorId = j.at("competitorId").get<string>();
    e.competitorName = j.at("competitorName").get<string>();
    e.status = j.at("status").get<string>();
    e.postCompStatus = j.at("postCompStatus").get<string>();
    e.gender = j.at("gender").get<string>();
    e.age = j.at("age").get<string>();
    e.height = j.at("height").get<string>();
    e.weight = j.at("weight").get<string>();
}

struct Score {
    size_t ordinal;
    string rank;
    string score;
    string scoreDisplay;
    string scaled;
    optional<size_t> time;
    string breakdown;
};

void from_json(const json& j, Score& s)
{
    s.ordinal = j.at("ordinal").get<size_t>();
    s.rank = j.at("rank").get<string>();
    s.score = j.at("score").get<string>();
    s.scoreDisplay = j.at("scoreDisplay").get<string>();
    s.scaled = j.at("scaled").get<string>();
    s.time = j.find("time") == j.end() ? optional<size_t>{} : j.at("time").get<size_t>();
    s.breakdown = j.at("breakdown").get<string>();
}

struct Row {
    Entrant entrant;
    vector<Score> scores;
    string overallRank;
    string overallScore;
};

void from_json(const json& j, Row& r)
{
    r.entrant = j.at("entrant").get<Entrant>();
    r.scores = j.at("scores").get<vector<Score> >();
    r.overallRank = j.at("overallRank").get<string>();
    r.overallScore = j.at("overallScore").get<string>();
}

struct Page {
    Pagination pagination;
    vector<Row> leaderboardRows;
};

void from_json(const json& j, Page& p)
{
    p.pagination = j.at("pagination").get<Pagination>();
    p.leaderboardRows = j.at("leaderboardRows").get<vector<Row> >();
}

int main()
{
    try {
        auto myUrl = make_unique<curlpp::options::Url>(
            "https://games.crossfit.com/competitions/api/v1/competitions/open/2018/"
            "leaderboards?page=4");
        curlpp::Easy myRequest;
        myRequest.setOpt(move(myUrl));
        std::ostringstream os;
        curlpp::options::WriteStream ws(&os);
        myRequest.setOpt(ws);
        myRequest.perform();
        const auto j3 = json::parse(os.str());
        const auto page = j3.get<Page>();
        cout << j3.dump();
    } catch (curlpp::RuntimeError& e) {
        cout << e.what() << endl;
    } catch (curlpp::LogicError& e) {
        cout << e.what() << endl;
    }

    return 0;
}
