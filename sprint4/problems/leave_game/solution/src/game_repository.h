#pragma once

#include "irepository.h"
#include "ConnectionPool.h"

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
    std::string GetResults(){
        std::string results;

        return results;
    }
    ~GameRepository() {}
private:
    ConnectionPool pool_;
};