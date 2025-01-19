#pragma once
#include <optional>
#include <vector>
#include <string>
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
// #include "../postgres/postgres.h"

namespace app {


class UnitOfWork {
public:
    virtual std::string AddAuthor(const std::string& name) = 0;
    virtual bool DeleteAuthor(const std::string& aid_str) = 0;
    virtual std::vector<domain::Author> GetAuthors() = 0;
    virtual void AddBook(const int year, const std::string& title,  const std::string& aid, std::vector<std::string>& tags) = 0;
    virtual std::vector<domain::BookRepr> GetBooks(std::string_view param) = 0;
    virtual std::vector<domain::BookRepr> GetAuthorBooks(const std::string& aid) = 0;
    virtual std::optional<domain::Author> GetAuthorByName(std::string_view name) = 0;
    virtual void RenameAuthor(std::string_view aid_str, std::string_view name) = 0;
    virtual void DeleteBook(std::string_view bid) = 0;
    virtual void EditBook(const domain::BookRepr& book) = 0;

    virtual ~UnitOfWork() = default;
protected:
};

class UnitOfWorkFactory {
public:
    UnitOfWorkFactory(UnitOfWork& unit) : unit_(unit){}
    UnitOfWork& Get() {
        return unit_;
    }
    const UnitOfWork& Get() const {
        return unit_;
    }
private:
    UnitOfWork& unit_;
};

}