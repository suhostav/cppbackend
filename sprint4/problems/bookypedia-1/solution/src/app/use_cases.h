#pragma once

#include <string>
#include <vector>
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"

namespace app {

class UseCases {
public:
    virtual void AddAuthor(const std::string& name) = 0;
    virtual std::vector<domain::Author> GetAuthors() const = 0;
    virtual void AddBook(int year, const std::string& title, const std::string& aid) = 0;
    virtual std::vector<domain::BookRepr> GetBooks() const = 0;
    virtual std::vector<domain::BookRepr> GetAuthorBooks(const std::string& aid) = 0;
protected:
    ~UseCases() = default;
};

}  // namespace app
