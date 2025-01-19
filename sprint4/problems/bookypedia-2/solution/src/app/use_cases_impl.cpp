#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

std::string UseCasesImpl::AddAuthor(const std::string& name) {
    auto& uwork = factory_.Get();
    return uwork.AddAuthor(name);
}

bool UseCasesImpl::DeleteAuthor(const std::string& aid_str){
    auto& uwork = factory_.Get();
    return uwork.DeleteAuthor(aid_str);
}

std::vector<Author> UseCasesImpl::GetAuthors() {
    auto& uwork = factory_.Get();
    return uwork.GetAuthors();
}

std::optional<domain::Author> UseCasesImpl::GetAuthorByName(std::string_view name) {
    auto& uwork = factory_.Get();
    return uwork.GetAuthorByName(name);
}

void UseCasesImpl::RenameAuthor(std::string_view aid_str, std::string_view name){
    auto& uwork = factory_.Get();
    return uwork.RenameAuthor(aid_str, name);
}

void UseCasesImpl::AddBook(const int year, const std::string& title,  const std::string& aid, std::vector<std::string>& tags) {
    auto& uwork = factory_.Get();
    uwork.AddBook(year, title, aid, tags);
}

std::vector<domain::BookRepr> UseCasesImpl::GetBooks(std::string_view param) {
    auto& uwork = factory_.Get();
    return uwork.GetBooks(param);
}

std::vector<domain::BookRepr> UseCasesImpl::GetAuthorBooks(const std::string& aid) {
    auto& uwork = factory_.Get();
    return uwork.GetAuthorBooks(aid);
}

void UseCasesImpl::DeleteBook(std::string_view bid){
    auto& uwork = factory_.Get();
    return uwork.DeleteBook(bid);
}

void UseCasesImpl::EditBook(const domain::BookRepr& book){
    auto& uwork = factory_.Get();
    return uwork.EditBook(book);
}

}  // namespace app
