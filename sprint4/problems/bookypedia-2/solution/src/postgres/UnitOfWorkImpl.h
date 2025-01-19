#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>
#include <pqxx/pqxx>
#include <string>
#include <vector>

#include "../domain/author.h"
#include "../domain/book.h"
#include "../app/UntiOfWork.h"
#include "postgres.h"
#include "../util/tagged_uuid.h"

namespace postgres {

class UnitOfWorkImpl : public app::UnitOfWork {
public:
    UnitOfWorkImpl(const std::string& conn_str) : database_(pqxx::connection(conn_str)) {  }
    pqxx::work StartTrasaction() { return database_.StartTransaction(); }
    void Commit(pqxx::work& work) {work.commit(); }

    std::string AddAuthor(const std::string& name) override;
    bool DeleteAuthor(const std::string& aid_str) override;
    std::vector<domain::Author> GetAuthors() override;
    void AddBook(const int year, const std::string& title,  const std::string& aid, std::vector<std::string>& tags) override;
    std::vector<domain::BookRepr> GetBooks(std::string_view param) override;
    std::vector<domain::BookRepr> GetAuthorBooks(const std::string& aid) override;
    std::optional<domain::Author> GetAuthorByName(const std::string_view name) override;
    void RenameAuthor(std::string_view aid_str, std::string_view name) override;
    void DeleteBook(std::string_view bid) override;
    void EditBook(const domain::BookRepr& book) override;

    pqxx::connection& GetConnection() { return database_.GetConnection();}
private:
    Database database_;
};
}