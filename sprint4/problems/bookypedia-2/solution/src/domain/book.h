#pragma once
#include <string>
#include <vector>
#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;

class Book {
public:
    Book(BookId id, AuthorId aid, std::string title, int year)
        : id_(std::move(id))
        , aid_(aid)
        , title_(std::move(title))
        , year_(year) {
    }

    const BookId& GetId() const noexcept {
        return id_;
    }

    const AuthorId& GetAuthorId() const noexcept {
        return aid_;
    }

    const std::string& GetTitle() const noexcept {
        return title_;
    }

    int GetYear() const noexcept {
        return year_;
    }

private:
    BookId id_;
    AuthorId aid_;
    std::string title_;
    int year_;
};

struct BookRepr {
    std::string title;
    std::string year;
};

class BookRepository {
public:
    virtual void Save(const Book& book) = 0;
    virtual std::vector<BookRepr> GetBooks() = 0;
    virtual std::vector<BookRepr> GetAuthorBooks(const std::string& aid) = 0;

protected:
    ~BookRepository() = default;
};

}  // namespace domain
