#include <catch2/catch_test_macros.hpp>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"
#include "../src/domain/book.h"

namespace {

struct MockAuthorRepository : domain::AuthorRepository {
    std::vector<domain::Author> saved_authors;

    void Save(const domain::Author& author) override {
        saved_authors.emplace_back(author);
    }
    std::vector<domain::Author> GetAuthors() override {
        return saved_authors;
    }
};

struct MockBookRepository : domain::BookRepository {
    std::vector<domain::Book> saved_books;
    void Save(const domain::Book& book) override {
        saved_books.emplace_back(book);
    }

    std::vector<domain::BookRepr> GetBooks() override {
        std::vector<domain::BookRepr> books;
        for(const auto& book : saved_books){
            domain::BookRepr br{.title = book.GetTitle(), .year = std::to_string(book.GetYear())};
            books.push_back(br);
        }
        return books;
    }

    std::vector<domain::BookRepr> GetAuthorBooks(const std::string& aid) override{
        std::vector<domain::BookRepr> books;
        for(const auto& book : saved_books){
            std::string baid{book.GetId().ToString()};
            if(baid == aid){
                domain::BookRepr br{.title = book.GetTitle(), .year = std::to_string(book.GetYear())};
                books.push_back(br);
            }
        }
        return books;
    }

};

struct Fixture {
    MockAuthorRepository authors;
    MockBookRepository books;
};

}  // namespace

SCENARIO_METHOD(Fixture, "Book Adding") {
    GIVEN("Use cases") {
        app::UseCasesImpl use_cases{authors, books};

        WHEN("Adding an author") {
            const auto author_name = "Joanne Rowling";
            use_cases.AddAuthor(author_name);

            THEN("author with the specified name is saved to repository") {
                REQUIRE(authors.saved_authors.size() == 1);
                CHECK(authors.saved_authors.at(0).GetName() == author_name);
                CHECK(authors.saved_authors.at(0).GetId() != domain::AuthorId{});
            }
        }
        WHEN("Adding book") {
            const int y = 1999;
            const std::string book_title{"Book1"};
            domain::AuthorId aid{domain::AuthorId::New()};
            use_cases.AddBook(y, book_title, aid.ToString());

            THEN("book with specified name is saved?") {
                REQUIRE(books.saved_books.size() == 1);
                CHECK(books.saved_books.at(0).GetTitle() == book_title);
            }
        }
        WHEN("Getting all books"){
            const int y = 1999;
            const std::string book_title{"Book1"};
            domain::AuthorId aid{domain::AuthorId::New()};
            use_cases.AddBook(y, book_title, aid.ToString());

            auto brs = use_cases.GetBooks();
            REQUIRE(brs.size() == 1);
            CHECK(books.saved_books.at(0).GetTitle() == brs[0].title);
        }
    }
}