#include "postgres.h"

#include <iostream>
#include <sstream>

#include <pqxx/pqxx>
#include <pqxx/zview.hxx>

#include <boost/uuid/uuid.hpp>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void AuthorRepositoryImpl::Save(const domain::Author& author) {
    // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
    // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
    // запросов выполнить в рамках одной транзакции.
    // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
    work.commit();
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    pqxx::work work{connection_};
    work.exec_params(
        R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4);)"_zv,
        book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetYear());
    work.commit();
}

std::vector<domain::Author> AuthorRepositoryImpl::GetAuthors() {
    pqxx::read_transaction r(connection_);
    std::vector<domain::Author> authors;
    pqxx::result result = r.exec("SELECT id, name FROM authors ORDER BY name;");
    for (auto const &row: result){
        std::string fields[2];
        size_t i = 0;
        for (auto const &field: row) {
            fields[i++] = field.c_str();
        }
        util::detail::UUIDType uuid = util::detail::UUIDFromString(fields[0]);
        domain::AuthorId id(uuid);
        authors.emplace_back(id, fields[1]);
        // authors.emplace_back(fields[0], fields[1]);
    }
    return authors;
}

std::vector<domain::BookRepr> BookRepositoryImpl::GetBooks(){
    std::vector<domain::BookRepr> books;
    pqxx::read_transaction r(connection_);
    pqxx::result result = r.exec("SELECT title, publication_year  FROM books ORDER BY title;");
    for (auto const &row: result){
        std::string fields[2];
        size_t i = 0;
        for (auto const &field: row) {
            fields[i++] = field.c_str();
        }
        books.emplace_back(fields[0], fields[1]);
    }
    return books;
}

std::vector<domain::BookRepr> BookRepositoryImpl::GetAuthorBooks(const std::string& aid) {
    std::vector<domain::BookRepr> books;
    pqxx::read_transaction r(connection_);
    std::stringstream ss;
    ss << "SELECT title, publication_year  FROM books";
    if(!aid.empty()){
        ss << " WHERE author_id='" << aid;
        ss << "' ORDER BY publication_year";
    } else {
        ss << " ORDER BY title";
    }
    ss << ";";
    std::string query{ss.str()};
    try {
        pqxx::result result = r.exec(query);
        for (auto const &row: result){
            std::string fields[2];
            size_t i = 0;
            for (auto const &field: row) {
                fields[i++] = field.c_str();
            }
            books.emplace_back(fields[0], fields[1]);
        }
    } catch(std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return books;
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);
    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id UUID CONSTRAINT books_id_constraint PRIMARY KEY,
    author_id UUID NOT NULL,
    title varchar(100) NOT NULL,
    publication_year integer
);
)"_zv);
    // коммитим изменения
    work.commit();
}

}  // namespace postgres