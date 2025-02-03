#pragma once

#include "irepository.h"
#include "ConnectionPool.h"
#include <pqxx/result>
#include <boost/json.hpp>

using pqxx::operator""_zv;

class GameRepository : public repository {
public:
    explicit GameRepository(unsigned threads_num, std::string conn_str) :
        pool_(threads_num, [&conn_str](){ return std::make_shared<pqxx::connection>(conn_str); }) {
        CreateTable();
    }
    void CreateTable() override {
        ConnectionPool::ConnectionWrapper conn = pool_.GetConnection();
        pqxx::work work{*conn};
        work.exec(R"(
            CREATE TABLE IF NOT EXISTS retired_players (
                id SERIAL PRIMARY KEY,
                name varchar(100) NOT NULL,
                score integer NOT NULL,
                total_seconds integer NOT NULL
            );)"_zv);
        work.exec("CREATE INDEX IF NOT EXISTS sort_index ON retired_players (score DESC, total_seconds, name);"_zv);
        work.commit();

    }
    void AddResult(const model::Dog& dog) override {
        ConnectionPool::ConnectionWrapper conn = pool_.GetConnection();
        pqxx::work work{*conn};
        work.exec_params(R"(
            INSERT INTO retired_players (name, score, total_seconds) VALUES ($1, $2, $3)
        )"_zv, dog.GetName(), dog.GetScore(), dog.GetTotalSeconds());
        work.commit();
    }
    std::string GetResults(int start, int32_t max_items){
        boost::json::array jrows;
        ConnectionPool::ConnectionWrapper conn = pool_.GetConnection();
        pqxx::read_transaction r(*conn);
        auto result = r.exec_params("SELECT name, score, total_seconds FROM retired_players ORDER BY score DESC, total_seconds, name OFFSET %1 limit %2"_zv
            , start, max_items);
        for( auto row : result) {
            boost::json::object jrow;
            jrow["name"] = row[0].as<std::string>();
            jrow["score"] = row[1].as<int>();
            jrow["total_seconds"] = row[2].as<int>();
            jrows.push_back(jrow);
        }
        return boost::json::serialize(jrows);
    }
    ~GameRepository() {}
private:
    ConnectionPool pool_;
};