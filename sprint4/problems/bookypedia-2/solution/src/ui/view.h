#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>
#include "../domain/author.h"
#include "../domain/book.h"

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
    std::string title;
    std::string author_id;
    int publication_year = 0;
};

struct AuthorInfo {
    std::string id;
    std::string name;
};

// struct BookInfo {
//     std::string id;
//     std::string title;
//     int publication_year;
//     std::string author;
// };

}  // namespace detail

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input) const;
    bool AddBook(std::istream& cmd_input) const;
    bool ShowAuthors() const;
    bool ShowBook(std::istream& cmd_input) const;
    bool ShowBooks() const;
    bool ShowAuthorBooks() const;
    bool DeleteAuthor(std::istream& cmd_input) const;
    bool EditAuthor(std::istream& cmd_input) const;
    bool DeleteBook(std::istream& cmd_input) const;
    bool EditBook(std::istream& cmd_input) const;

    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input) const;
    std::optional<std::string> SelectAuthor() const;
    // std::optional<domain::BookRepr> SelectBook(std::string_view param) const:
    std::string SelectAuthorFromList() const;
    std::vector<detail::AuthorInfo> GetAuthors() const;
    std::vector<domain::BookRepr> GetBooks(std::string_view param = "") const;
    std::vector<domain::BookRepr> GetAuthorBooks(const std::string& author_id) const;

    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace ui