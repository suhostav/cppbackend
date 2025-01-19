#include "postgres.h"
#include <boost/algorithm/string/trim.hpp>

#include <iostream>
#include <sstream>

#include <pqxx/pqxx>
#include <pqxx/zview.hxx>

#include <boost/uuid/uuid.hpp>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

std::string AuthorRepositoryImpl::Save(const domain::Author& author) {
    if(work_ == nullptr){
        throw std::runtime_error("Save author: no unit of work!");
    }
    pqxx::work& work{*work_};
    auto result = work.exec_params(
        R"(
INSERT INTO authors (id, name) VALUES ($1, $2)
ON CONFLICT (id) DO UPDATE SET name=$2;
)"_zv,
        author.GetId().ToString(), author.GetName());
    return author.GetId().ToString();
    // work.commit();
}

void BookRepositoryImpl::Save(const domain::Book& book) {
    if(work_ == nullptr){
        throw std::runtime_error("Save author: no unit of work!");
    }
    pqxx::work& work{*work_};
    work.exec_params(
        R"(
INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4);)"_zv,
        book.GetId().ToString(), book.GetAuthorId().ToString(), book.GetTitle(), book.GetYear());
    for(auto& tag : book.GetTags()){
        work.exec_params(
            R"(
    INSERT INTO book_tags (book_id, tag) VALUES ($1, $2);)"_zv,
            book.GetId().ToString(), tag);
    }
    // work.commit();
}

bool AuthorRepositoryImpl::Delete(const std::string& aid_str){
    if(work_ == nullptr){
        throw std::runtime_error("Delete author: no unit of work!");
    }
    pqxx::work& work{*work_};
    auto result = work.exec_params(R"(DELETE FROM authors WHERE id=$1)"_zv, aid_str);
    if(std::size(result) == 0) {
        return false;
    }
    return true;
}

void BookRepositoryImpl::DeleteAuthorBooks(const std::string& aid_str){
    if(work_ == nullptr){
        throw std::runtime_error("Delete author: no unit of work!");
    }
    pqxx::work& work{*work_};
    auto result = work.exec_params(R"(SELECT id FROM books WHERE author_id=$1)"_zv, aid_str);
    work.exec_params(R"(DELETE FROM books WHERE author_id=$1)"_zv, aid_str);
    for(auto row : result){
        auto book_id = row[0].view();
        work.exec_params(R"(DELETE FROM book_tags WHERE book_id=$1)"_zv, book_id);
    }
}

void BookRepositoryImpl::DeleteBook(std::string_view bid){
    if(work_ == nullptr){
        throw std::runtime_error("Delete book: no unit of work!");
    }
    pqxx::work& work{*work_};
    work.exec_params(R"(DELETE FROM books WHERE id=$1)"_zv, bid);
    work.exec_params(R"(DELETE FROM book_tags WHERE book_id=$1)"_zv, bid);
}

void BookRepositoryImpl::EditBook(const domain::BookRepr& book) {
    if(work_ == nullptr){
        throw std::runtime_error("Edit book: no unit of work!");
    }
    pqxx::work& work{*work_};
    work.exec_params(R"(UPDATE books SET title=$1, publication_year=$2 WHERE id=$3)"_zv, book.title, book.year, book.id);
    work.exec_params(R"(DELETE FROM book_tags WHERE book_id=$1)"_zv, book.id);
    for(auto& tag : book.tags){
        work.exec_params(R"(INSERT INTO book_tags (book_id, tag) VALUES ($1, $2))"_zv, book.id, tag);
    }
}

std::vector<domain::Author> AuthorRepositoryImpl::GetAuthors() {
    if(read_tr_ == nullptr){
        throw std::runtime_error("GetAuthors: no unit of work!");
    }
    pqxx::read_transaction& r(*read_tr_);
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

std::optional<domain::Author> AuthorRepositoryImpl::GetAuthorByName(const std::string_view name){
    if(read_tr_ == nullptr){
        throw std::runtime_error("GetAuthors: no unit of work!");
    }
    pqxx::read_transaction& r(*read_tr_);
    pqxx::result result = r.exec_params("SELECT id, name FROM authors WHERE name=$1;"_zv, name);
    if(std::size(result) == 0){
        return {};
    }
    util::detail::UUIDType uuid = util::detail::UUIDFromString(result[0][0].view());
    domain::AuthorId id(uuid);
    std::string name_str{result[0][1].view()};
    return domain::Author(id, name_str);
}

void AuthorRepositoryImpl::RenameAuthor(std::string_view aid_str, std::string_view name) {
    if(work_ == nullptr){
        throw std::runtime_error("Delete author: no unit of work!");
    }
    pqxx::work& work{*work_};
    auto result = work.exec_params(R"(UPDATE authors SET name=$2 WHERE id=$1)"_zv, aid_str, name);

}

std::vector<domain::BookRepr> BookRepositoryImpl::GetBooks(std::string_view param){
    param = util::Trim(param);
    std::vector<domain::BookRepr> books;
    if(read_tr_ == nullptr){
        throw std::runtime_error("GetBooks: no unit of work!");
    }
    pqxx::read_transaction& r(*read_tr_);
    pqxx::result result;
    if(param.empty()) {
        result = r.exec("SELECT books.id, books.title, books.publication_year, books.author_id, authors.name  FROM books "
            "JOIN authors ON books.author_id=authors.id ORDER BY books.title, authors.name, books.publication_year;");
    } else {
        std::string q{"SELECT books.id, books.title, books.publication_year, books.author_id, authors.name  FROM books "
            "JOIN authors ON books.author_id=authors.id  WHERE (title LIKE '"};
        q += param;
        q += "%') ORDER BY books.title, authors.name, books.publication_year;";
        // result = r.exec_params("SELECT id, title, publication_year, author_id  FROM books WHERE (title LIKE '$1%') ORDER BY title;"_zv, param);
        result = r.exec(q);
    }
    for (auto const &row: result){
        std::string fields[5];
        size_t i = 0;
        for (auto const &field: row) {
            fields[i++] = field.c_str();
        }
        pqxx::result tags_result = r.exec_params("SELECT tag FROM book_tags WHERE book_id=$1 ORDER BY tag;"_zv, fields[0]);
        std::vector<std::string> tags;
        for(auto const &t : tags_result){
            tags.emplace_back(std::string(t[0].view()));
        }
        books.emplace_back(fields[0], fields[1], fields[2], fields[3], fields[4], tags);
    }
    return books;
}

std::vector<domain::BookRepr> BookRepositoryImpl::GetAuthorBooks(const std::string& aid) {
    std::vector<domain::BookRepr> books;
    if(read_tr_ == nullptr){
        throw std::runtime_error("GetAuthorBooks: no unit of work!");
    }
    pqxx::read_transaction& r(*read_tr_);
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
    // pqxx::work work{connection_};
    pqxx::work work = StartTransaction();
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
    work.exec(R"(
CREATE TABLE IF NOT EXISTS book_tags (
    book_id UUID,
    tag varchar(30)
);
)"_zv);
    // коммитим изменения
    work.commit();
}

}  // namespace postgres