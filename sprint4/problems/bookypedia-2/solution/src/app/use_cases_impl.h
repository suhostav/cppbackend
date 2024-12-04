#pragma once
#include <vector>
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(domain::AuthorRepository& authors, domain::BookRepository& books)
        : authors_{authors}
        , books_{books} {
    }

    void AddAuthor(const std::string& name) override;
    std::vector<domain::Author> GetAuthors() const override;
    void AddBook(int year, const std::string& title, const std::string& aid) override;
    std::vector<domain::BookRepr> GetBooks() const override;
    std::vector<domain::BookRepr> GetAuthorBooks(const std::string& aid) override;
private:
    domain::AuthorRepository& authors_;
    domain::BookRepository& books_;
};

}  // namespace app
