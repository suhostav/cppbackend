#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::AddAuthor(const std::string& name) {
    authors_.Save({AuthorId::New(), name});
}

std::vector<Author> UseCasesImpl::GetAuthors() const {
    return authors_.GetAuthors();
}

void UseCasesImpl::AddBook(const int year, const std::string& title,  const std::string& aid) {
    books_.Save({BookId::New(), AuthorId::FromString(aid), title, year});
}

std::vector<domain::BookRepr> UseCasesImpl::GetBooks() const {
    return books_.GetBooks();
}

std::vector<domain::BookRepr> UseCasesImpl::GetAuthorBooks(const std::string& aid) {
    return books_.GetAuthorBooks(aid);
}

}  // namespace app
