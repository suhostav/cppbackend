#pragma once
#include <vector>
#include "../domain/author_fwd.h"
#include "../domain/book_fwd.h"
#include "UntiOfWork.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(UnitOfWorkFactory& factory)
        : factory_{factory}{
    }

    std::string AddAuthor(const std::string& name) override;
    bool DeleteAuthor(const std::string& aid_str) override;
    std::vector<domain::Author> GetAuthors() override;
    void AddBook(int year, const std::string& title, const std::string& aid, std::vector<std::string>& tags) override;
    std::vector<domain::BookRepr> GetBooks(std::string_view param) override;
    std::vector<domain::BookRepr> GetAuthorBooks(const std::string& aid) override;
    std::optional<domain::Author> GetAuthorByName(std::string_view name) override;
    void RenameAuthor(std::string_view aid_str, std::string_view name) override;
    void DeleteBook(std::string_view bid) override;
    void EditBook(const domain::BookRepr& book) override;
private:
    UnitOfWorkFactory& factory_;
};

}  // namespace app
