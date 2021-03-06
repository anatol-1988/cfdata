#pragma once
#include <experimental/optional>
#include <nlohmann/json.hpp>
#include <string>

namespace entities
{

using std::string;
using std::vector;
using std::experimental::optional;
using std::experimental::nullopt;
using nlohmann::json;

struct Pagination final {
    size_t currentPage;
    size_t totalPages;
    size_t totalCompetitors;
};

void from_json(json const &j, Pagination &p)
{
    p.currentPage = j.at("currentPage").get<size_t>();
    p.totalPages = j.at("totalPages").get<size_t>();
    p.totalCompetitors = j.at("totalCompetitors").get<size_t>();
}

struct Entrant final {
    string competitorId;
    string competitorName;
    string status;
    string postCompStatus;
    string gender;
    string divisionId;
    string age;
    string height;
    string weight;
};

void from_json(json const &j, Entrant &e)
{
    e.competitorId = j.at("competitorId").get<string>();
    e.competitorName = j.at("competitorName").get<string>();
    e.status = j.at("status").get<string>();
    e.postCompStatus = j.at("postCompStatus").get<string>();
    e.gender = j.at("gender").get<string>();
    e.divisionId = j.at("divisionId").get<string>();
    e.age = j.at("age").get<string>();
    e.height = j.at("height").get<string>();
    e.weight = j.at("weight").get<string>();
}

struct Score final {
    size_t ordinal;
    string rank;
    string score;
    string scoreDisplay;
    string scaled;
    optional<size_t> time;
    optional<string> breakdown;
};

void from_json(json const &j, Score &s)
{
    s.ordinal = j.at("ordinal").get<size_t>();
    s.rank = j.at("rank").get<string>();
    s.score = j.at("score").get<string>();
    s.scoreDisplay = j.at("scoreDisplay").get<string>();
    s.scaled = j.at("scaled").get<string>();

    if (j.find("time") == j.end())
        s.time = nullopt;
    else if (j.at("time").is_number_unsigned())
        s.time = j.at("time").get<size_t>();
    else if (j.at("time").is_string()) {
        auto const str = j.at("time").get<string>();

        if (!str.empty())
            s.time = stoul(str);
    }

    s.breakdown = j.find("breakdown") == j.end()
        ? optional<string>{}
        : j.at("breakdown").get<string>();
}

struct Row final {
    Entrant entrant;
    vector<Score> scores;
    optional<string> overallRank;
    optional<string> overallScore;
};

void from_json(json const &j, Row &r)
{
    r.entrant = j.at("entrant").get<Entrant>();
    r.scores = j.at("scores").get<vector<Score>>();
    r.overallRank = j.find("overallRank") != cend(j)
        ? j.at("overallRank").get<string>()
        : optional<string>{};
    r.overallScore = j.find("overallScore") != cend(j)
        ? j.at("overallScore").get<string>()
        : optional<string>{};
}

struct Page final {
    Pagination pagination;
    vector<Row> leaderboardRows;
};

void from_json(json const &j, Page &p)
{
    p.pagination = j.at("pagination").get<Pagination>();
    p.leaderboardRows = j.at("leaderboardRows").get<vector<Row>>();
}
}
