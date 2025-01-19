#include <catch2/catch_test_macros.hpp>
// #include <pqxx/connection>
// #include <pqxx/transaction>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"
#include "../src/domain/book.h"
#include "../src/app/UntiOfWork.h"
namespace {

struct MockAuthorRepository : domain::AuthorRepository {
    std::vector<domain::Author> saved_authors;

    std::string Save(const domain::Author& author) override {
        saved_authors.emplace_back(author);
        return {};
    }
    bool Delete(const std::string& aid_str) override{
        return true;
    }
    std::vector<domain::Author> GetAuthors() override {
        return saved_authors;
    }
    std::vector<domain::Author> GetAuthors() const {
        return saved_authors;
    }
    std::optional<domain::Author> GetAuthorByName(const std::string_view name) override {
        return {};
    }
    void RenameAuthor(std::string_view aid_str, std::string_view name) override {

    }
};

struct MockBookRepository : domain::BookRepository {
    std::vector<domain::Book> saved_books;
    void Save(const domain::Book& book) override {
        saved_books.emplace_back(book);
    }

    void DeleteAuthorBooks(const std::string& aid_str) override{

    }

    std::vector<domain::BookRepr> GetBooks(std::string_view param = "") override {
        std::vector<domain::BookRepr> books;
        for(const auto& book : saved_books){
            domain::BookRepr br{.title = book.GetTitle(), .year = std::to_string(book.GetYear())};
            books.push_back(br);
        }
        return books;
    }

    std::vector<domain::BookRepr> GetBooks() const {
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

    std::vector<domain::BookRepr> GetAuthorBooks(const std::string& aid) const{
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

    void DeleteBook(std::string_view bid){

    }

    void EditBook(const domain::BookRepr& book) override{}
};


class MockUnitOfWork : public app::UnitOfWork{
public:
    MockUnitOfWork() {  }

    std::string AddAuthor(const std::string& name) override {
        return authors.Save(domain::Author(domain::AuthorId::New(), name));
    }
    bool DeleteAuthor(const std::string& aid_str) override {
        return false;
    }
    std::vector<domain::Author> GetAuthors() override { return authors.GetAuthors();}
    void AddBook(const int year, const std::string& title,  const std::string& aid, std::vector<std::string>& tags) override {
        domain::AuthorId autor_id(util::detail::UUIDFromString(aid));
        domain::Book book{ domain::BookId::New(), autor_id, title, year, tags};
        books.Save(book);
    }
    std::vector<domain::BookRepr> GetBooks(std::string_view param) override { return books.GetBooks();}
    std::vector<domain::BookRepr> GetAuthorBooks(const std::string& aid) override {return books.GetAuthorBooks(aid);}
    std::optional<domain::Author> GetAuthorByName(std::string_view name) { return {}; }
    void RenameAuthor(std::string_view aid_str, std::string_view name) override {}
    void DeleteBook(std::string_view bid) override {}
    void EditBook(const domain::BookRepr& book) override { books.EditBook(book);}
protected:
    MockAuthorRepository authors;
    MockBookRepository books;
};



struct Fixture {
    // MockUnitOfWork uwork;
    // app::UnitOfWorkFactory factory(uwork);
    // app::UseCasesImpl use_cases{factory};

};

}  // namespace

SCENARIO_METHOD(Fixture, "Book Adding") {
    GIVEN("Use cases") {
    MockUnitOfWork uwork;
    app::UnitOfWorkFactory factory(uwork);
    app::UseCasesImpl use_cases{factory};

        WHEN("Adding an author") {
            const auto author_name = "Joanne Rowling";
            use_cases.AddAuthor(author_name);

            THEN("author with the specified name is saved to repository") {
                REQUIRE(use_cases.GetAuthors().size() == 1);
                CHECK(use_cases.GetAuthors().at(0).GetName() == author_name);
                CHECK(use_cases.GetAuthors().at(0).GetId() != domain::AuthorId{});
            }
        }
        WHEN("Adding book") {
            const int y = 1999;
            const std::string book_title{"Book1"};
            domain::AuthorId aid{domain::AuthorId::New()};
            std::vector<std::string> tags;
            use_cases.AddBook(y, book_title, aid.ToString(), tags);

            THEN("book with specified name is saved?") {
                REQUIRE(use_cases.GetBooks("").size() == 1);
                CHECK(use_cases.GetBooks("").at(0).title == book_title);
            }
        }
        WHEN("Getting all books"){
            const int y = 1999;
            const std::string book_title{"Book1"};
            domain::AuthorId aid{domain::AuthorId::New()};
            std::vector<std::string> tags;
            use_cases.AddBook(y, book_title, aid.ToString(), tags);

            auto brs = use_cases.GetBooks("");
            REQUIRE(brs.size() == 1);
            CHECK(use_cases.GetBooks("").at(0).title == brs[0].title);
        }
    }
}