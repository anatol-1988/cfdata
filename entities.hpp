#pragma once
#include <string>
#include <experimental/optional>
#include <nlohmann/json.hpp>

namespace entities {

using std::string;
using std::vector;
using std::experimental::optional;
using nlohmann::json;

struct Pagination {
    size_t currentPage;
    size_t totalPages;
    size_t totalCompetitors;
};

void from_json(json const &j, Pagination& p)
{
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

void from_json(json const &j, Entrant& e)
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

void from_json(json const &j, Score& s)
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

void from_json(json const &j, Row& r)
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

void from_json(json const &j, Page& p)
{
    p.pagination = j.at("pagination").get<Pagination>();
    p.leaderboardRows = j.at("leaderboardRows").get<vector<Row> >();
}
}
