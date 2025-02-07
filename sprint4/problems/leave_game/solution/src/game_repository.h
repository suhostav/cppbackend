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
        )"_zv, dog.GetName(), dog.GetScore(), dog.GetTotalTimeInt());
        work.commit();
    }
    std::string GetResults(int start, int32_t max_items){
        boost::json::array jrows;
        ConnectionPool::ConnectionWrapper conn = pool_.GetConnection();
        pqxx::read_transaction r(*conn);
        try{
            std::string query{"SELECT name, score, total_seconds FROM retired_players ORDER BY score DESC, total_seconds, name OFFSET "};
            query += std::to_string(start);
            query += " LIMIT ";
            query += std::to_string(max_items);
            query += ';';
            auto result = r.exec(query);
            for( auto row : result) {
                boost::json::object jrow;
                jrow["name"] = row[0].as<std::string>();
                jrow["score"] = row[1].as<int>();
                jrow["playTime"] = static_cast<double>(row[2].as<int>()) / 1000.0;
                jrows.push_back(jrow);
            }
        } catch(const std::exception& ex){
            std::cerr << ex.what() << std::endl;
        }
        return boost::json::serialize(jrows);
    }
    ~GameRepository() {}
private:
    ConnectionPool pool_;
};