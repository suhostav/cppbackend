#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>
#include <string>
#include <vector>

#include "../domain/author.h"
#include "../domain/book.h"
#include "../util/utils.h"

namespace postgres {

class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    explicit AuthorRepositoryImpl()  {
    }
    void SetWork(pqxx::work *work){
        work_ = work;
    }
    void SetReadTransaction(pqxx::read_transaction *rt){
        read_tr_ = rt;
    }
    std::string Save(const domain::Author& author) override;
    bool Delete(const std::string& aid_str) override;
    std::vector<domain::Author> GetAuthors() override;
    std::optional<domain::Author> GetAuthorByName(const std::string_view name) override;
    void RenameAuthor(std::string_view aid_str, std::string_view name) override;
    void Commit(){
        work_ = nullptr;
        read_tr_ = nullptr;
    }
private:
    pqxx::work *work_ = nullptr;
    pqxx::read_transaction *read_tr_ = nullptr;
};

class BookRepositoryImpl : public domain::BookRepository {
public:
    explicit BookRepositoryImpl(){
    }

    void SetWork(pqxx::work *work){
        work_ = work;
    }
    void SetReadTransaction(pqxx::read_transaction *rt){
        read_tr_ = rt;
    }
    void Save(const domain::Book& book) override;
    std::vector<domain::BookRepr> GetBooks(std::string_view param = "") override;
    std::vector<domain::BookRepr> GetAuthorBooks(const std::string& aid) override;
    void DeleteAuthorBooks(const std::string& aid_str) override;
    void DeleteBook(std::string_view bid) override;
    void EditBook(const domain::BookRepr& book) override;
    void Commit(){
        work_ = nullptr;
        read_tr_ = nullptr;
    }
private:
    pqxx::work *work_ = nullptr;
    pqxx::read_transaction *read_tr_ = nullptr;
};

class Database {
public:
    explicit Database(pqxx::connection connection);

    AuthorRepositoryImpl& GetAuthors() & {
        return authors_;
    }

    BookRepositoryImpl& GetBooks() & {
        return books_;
    }

    pqxx::connection& GetConnection() {
        return connection_;
    }

    const pqxx::connection& GetConnection() const {
        return connection_;
    }

    pqxx::work StartTransaction(){
        return pqxx::work{connection_};
    }

    void Commit(pqxx::work& work){
        work.commit();
    }

private:
    pqxx::connection connection_;
    AuthorRepositoryImpl authors_{};
    BookRepositoryImpl books_{};
};

}  // namespace postgres