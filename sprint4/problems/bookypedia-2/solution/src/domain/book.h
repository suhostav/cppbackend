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
    Book(BookId id, AuthorId aid, std::string title, int year, std::vector<std::string>& tags)
        : id_(std::move(id))
        , aid_(aid)
        , title_(std::move(title))
        , year_(year)
        , tags_(tags) {
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
    std::vector<std::string> GetTags() const{
        return tags_;
    }
    void SetTags(std::vector<std::string>& tags) {
        tags_ = tags;
    }

private:
    BookId id_;
    AuthorId aid_;
    std::string title_;
    int year_;
    std::vector<std::string> tags_;
};

struct BookRepr {
    std::string id;
    std::string title;
    std::string year;
    std::string author_id;
    std::string author_name;
    std::vector<std::string> tags;
};

class BookRepository {
public:
    virtual void Save(const Book& book) = 0;
    virtual std::vector<BookRepr> GetBooks(std::string_view param = "") = 0;
    virtual std::vector<BookRepr> GetAuthorBooks(const std::string& aid) = 0;
    virtual void DeleteAuthorBooks(const std::string& aid_str) = 0;
    virtual void DeleteBook(std::string_view bid) = 0;
    virtual void EditBook(const domain::BookRepr& book) = 0;
protected:
    ~BookRepository() = default;
};

}  // namespace domain
