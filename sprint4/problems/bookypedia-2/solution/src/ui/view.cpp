#include "view.h"

#include <boost/algorithm/string/trim.hpp>
#include <cassert>
#include <stdexcept>
#include <iostream>

#include "../app/use_cases.h"
#include "../menu/menu.h"
#include "../util/utils.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
    out << author.name;
    return out;
}

}  // namespace detail

std::ostream& operator<<(std::ostream& out, const domain::BookRepr& book) {
    out << book.title << " by " << book.author_name << ", " << book.year;
    return out;
}

template <typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " " << value << std::endl;
    }
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_{menu}
    , use_cases_{use_cases}
    , input_{input}
    , output_{output} {
    menu_.AddAction(  //
        "AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1)
        // ����
        // [this](auto& cmd_input) { return AddAuthor(cmd_input); }
    );
    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s,
                    std::bind(&View::AddBook, this, ph::_1));
    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
    menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s, std::bind(&View::ShowAuthorBooks, this));
    menu_.AddAction("DeleteAuthor"s, {}, "Delete author and his books and for every book all tags"s, std::bind(&View::DeleteAuthor, this, ph::_1));
    menu_.AddAction("EditAuthor"s, {}, "Edit author"s, std::bind(&View::EditAuthor, this, ph::_1));
    menu_.AddAction("ShowBook"s, {}, "Show book"s, std::bind(&View::ShowBook, this, ph::_1));
    menu_.AddAction("DeleteBook"s, {}, "Delete Book"s, std::bind(&View::DeleteBook, this, ph::_1));
    menu_.AddAction("EditBook"s, {}, "Edit Book"s, std::bind(&View::EditBook, this, ph::_1));
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        if(name.empty()){
            throw std::invalid_argument("");
        }
        use_cases_.AddAuthor(std::move(name));
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
    }
    return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        std::string aid_str;
        if(!name.empty()){
            auto autor_opt = use_cases_.GetAuthorByName(name);
            if(!autor_opt){
                output_ << "Failed to delete author\n";
                return true;
            }
            aid_str = autor_opt->GetId().ToString();
        } else {
            aid_str = SelectAuthorFromList();
        }
        if(aid_str.empty()){
            return true;
        }
        use_cases_.DeleteAuthor(std::move(aid_str));
    } catch (const std::exception&) {
        output_ << "Failed to delete author\n"sv << std::endl;
    }
    return true;
}

bool View::EditAuthor(std::istream& cmd_input) const {
    std::string name;
    std::string aid_str;
    std::getline(cmd_input, name);
    boost::algorithm::trim(name);
    if(name.empty()){
        aid_str = SelectAuthorFromList();
        if(aid_str.empty()){
            return false;
        }  
    } else {
        auto author_opt = use_cases_.GetAuthorByName(name);
        if(!author_opt){
            output_ << "Failed to edit author\n"s;
            return true;
        }
        auto aid = (*author_opt).GetId();
        aid_str = aid.ToString();
    }
    output_ << "Enter new name:"s << std::endl;
    std::string new_name;
    std::getline(input_, new_name);
    try{
    use_cases_.RenameAuthor(aid_str, new_name);
    } catch(...){
        output_ << "Failed to edit author\n"s;
    }
    return true;
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        if (auto params = GetBookParams(cmd_input)) {
            std::string tag_str;
            output_ << "Enter tags (comma separated):\n";
            std::getline(input_, tag_str);
            std::vector<std::string> tags = util::PrepareTags(tag_str);
            use_cases_.AddBook(params->publication_year, params->title, params->author_id, tags);
        }
    } catch (const std::exception&) {
        output_ << "Failed to add book"sv << std::endl;
    }
    return true;
}

bool View::ShowAuthors() const {
    PrintVector(output_, GetAuthors());
    return true;
}

bool View::ShowBooks() const {
    PrintVector(output_, GetBooks(""));
    return true;
}

bool View::ShowBook(std::istream& cmd_input) const {
    std::string title;
    std::getline(cmd_input, title);
    auto books = GetBooks(title);
    size_t ind = 0;
    if(books.size() == 0){
        return true;
    }
    if(books.size() > 1){
        PrintVector(output_, books);
        output_ << "Enter the book # or empty line to cancel:\n";
        std::string ind_str;
        std::getline(input_, ind_str);
        boost::algorithm::trim(ind_str);
        if(ind_str.empty() || !isdigit(ind_str[0])){
            return false;
        }
        ind = std::atoi(ind_str.c_str()) - 1;
    }
    auto& book = books[ind];
    output_ << "Title: " << book.title << std::endl;
    output_ << "Author: " << book.author_name << std::endl;
    output_ << "Publication year: " << book.year << std::endl;
    output_ << "Tags: " << util::Join(book.tags, ", ") << std::endl;
    return true;
}

bool View::DeleteBook(std::istream& cmd_input) const {
    std::string title;
    std::getline(cmd_input, title);
    auto books = GetBooks(title);
    size_t ind = 0;
    if(books.size() == 0){
        output_ << "Failed to delete book\n";
        return true;
    }
    if(books.size() > 1){
        PrintVector(output_, books);
        output_ << "Enter the book # or empty line to cancel:\n";
        std::string ind_str;
        std::getline(input_, ind_str);
        boost::algorithm::trim(ind_str);
        if(ind_str.empty() || !isdigit(ind_str[0])){
            return false;
        }
        ind = std::atoi(ind_str.c_str()) - 1;
    }
    auto& book = books[ind];
    try{
        use_cases_.DeleteBook(book.id);
    } catch(...){
        output_ << "Failed to delete book\n";
    }
    return true;
}

bool View::EditBook(std::istream& cmd_input) const{
    std::string title;
    std::getline(cmd_input, title);
    auto books = GetBooks(title);
    size_t ind = 0;
    if(books.size() == 0){
        output_ <<"Book not found\n";
        return true;
    }
    if(books.size() > 1){
        PrintVector(output_, books);
        output_ << "Enter the book # or empty line to cancel:\n";
        std::string ind_str;
        std::getline(input_, ind_str);
        boost::algorithm::trim(ind_str);
        if(ind_str.empty() || !isdigit(ind_str[0])){
            return true;
        }
        ind = std::atoi(ind_str.c_str()) - 1;
    }
    auto book = books[ind];
    output_ << "Enter new title or empty line to use the current one (" << book.title << "):\n";
    std::string new_title;
    std::getline(input_, new_title);
    if(!new_title.empty()){
        book.title = new_title;
    }
    output_ << "Enter publication year or empty line to use the current one (" << book.year << "):\n";
    std::string new_year;
    std::getline(input_, new_year);
    boost::algorithm::trim(new_year);
    if(!new_year.empty() || isdigit(new_year[0])){
        book.year = new_year;
    }
    output_ << "Enter tags (current tags: " << util::Join(book.tags, ", ")<< "):\n";
    std::string new_tags;
    std::getline(input_, new_tags);
    book.tags = util::PrepareTags(new_tags);
    use_cases_.EditBook(book);
    return true;
}

bool View::ShowAuthorBooks() const {
    // TODO: handle error
    try {
        if (auto author_id = SelectAuthor()) {
            PrintVector(output_, GetAuthorBooks(*author_id));
        }
    } catch (const std::exception&) {
        throw std::runtime_error("Failed to Show Books");
    }
    return true;
}

std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
    detail::AddBookParams params;

    cmd_input >> params.publication_year;
    std::getline(cmd_input, params.title);
    boost::algorithm::trim(params.title);

    auto author_id = SelectAuthor();
    if (not author_id.has_value())
        return std::nullopt;
    else {
        params.author_id = author_id.value();
        return params;
    }
}

std::optional<std::string> View::SelectAuthor() const {
    output_ << "Enter author name or empty line to select from list:\n";
    std::string author_str;
    std::getline(input_, author_str);
    // output_ << "Select author:" << std::endl;
    auto authors = GetAuthors();
    if(!author_str.empty()) {
        for(const auto& author : authors){
            if(author.name == author_str){
                return author.id;
            }
        }
        output_ << "No author found. Do you want to add " << author_str << " (y/n)?\n";
        std::string add_str;
        std::getline(input_, add_str);
        if(add_str == "y" || add_str == "Y"){
            return use_cases_.AddAuthor(author_str);
        } else {
            output_ << "Failed to add book\n";
            return {};
        }
    } else {
        PrintVector(output_, authors);
        output_ << "Enter author # or empty line to cancel" << std::endl;

        std::string str;
        if (!std::getline(input_, str) || str.empty()) {
            return std::nullopt;
        }
        int author_idx;
        try {
            author_idx = std::stoi(str);
        } catch (std::exception const&) {
            throw std::runtime_error("Invalid author num");
        }

        --author_idx;
        if (author_idx < 0 or author_idx >= authors.size()) {
            throw std::runtime_error("Invalid author num");
        }
        return authors[author_idx].id;
    }

}

// std::optional<domain::BookRepr> View::SelectBook(std::string_view param) const{
//     auto books = GetBooks2(title);
//     if(books.size() == 0){
//         return {}};
//     }
//     size_t ind = 0;
//     if(books.size() > 1){
//         PrintVector(output_, books);
//         std::string inp;
//         output_ << "Enter the book # or empty line to cancel:\n";
//         std::getline(input_, inp);
//         boost::algorithm::trim(inp);
//         if(inp,empty() || !isdigit(inp[0])){
//             return {};
//         }
//         ind = std::atoi(inp);
//     }

// }

std::string View::SelectAuthorFromList() const{
    auto authors = GetAuthors();
        PrintVector(output_, authors);
        output_ << "Enter author # or empty line to cancel" << std::endl;

        std::string str;
        if (!std::getline(input_, str) || str.empty()) {
            return {};
        }
        int author_idx;
        try {
            author_idx = std::stoi(str);
        } catch (std::exception const&) {
            throw std::runtime_error("Failed to edit author");
        }

        --author_idx;
        if (author_idx < 0 or author_idx >= authors.size()) {
            throw std::runtime_error("Failed to edit author");
        }
        return authors[author_idx].id;
}

std::vector<detail::AuthorInfo> View::GetAuthors() const {
    std::vector<detail::AuthorInfo> dst_autors;
    auto authors = use_cases_.GetAuthors();
    for(auto& author : authors){
        std::string id_str(author.GetId().ToString());
        dst_autors.emplace_back(id_str, author.GetName());
    }
    return dst_autors;
}

std::vector<domain::BookRepr> View::GetBooks(std::string_view param) const {
    return use_cases_.GetBooks(param);
}

std::vector<domain::BookRepr> View::GetAuthorBooks(const std::string& author_id) const {
    return use_cases_.GetAuthorBooks(author_id);;
}

}  // namespace ui
