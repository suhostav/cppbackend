#include "UnitOfWorkImpl.h"

namespace postgres {

std::string UnitOfWorkImpl::AddAuthor(const std::string& name){
    AuthorRepositoryImpl repo;
    auto work = StartTrasaction();
    repo.SetWork(&work);
    auto aid_str = repo.Save(domain::Author(domain::AuthorId::New(), name));
    Commit(work);
    return aid_str;
}

bool UnitOfWorkImpl::DeleteAuthor(const std::string& aid_str){
    AuthorRepositoryImpl auth_repo;
    BookRepositoryImpl book_repo;
    auto work = StartTrasaction();
    auth_repo.SetWork(&work);
    book_repo.SetWork(&work);
    bool result = auth_repo.Delete(aid_str);
    book_repo.DeleteAuthorBooks(aid_str);
    Commit(work);
    return result;
}

std::vector<domain::Author> UnitOfWorkImpl::GetAuthors() {
    AuthorRepositoryImpl repo;
    pqxx::read_transaction rt(GetConnection());
    repo.SetReadTransaction(&rt);
    return repo.GetAuthors();
}

std::optional<domain::Author> UnitOfWorkImpl::GetAuthorByName(const std::string_view name) {
    AuthorRepositoryImpl repo;
    pqxx::read_transaction rt(GetConnection());
    repo.SetReadTransaction(&rt);
    return repo.GetAuthorByName(name);
}

void UnitOfWorkImpl::RenameAuthor(std::string_view aid_str, std::string_view name){
    AuthorRepositoryImpl repo;
    auto work = StartTrasaction();
    repo.SetWork(&work);
    repo.RenameAuthor(aid_str, name);
    Commit(work);
}

void UnitOfWorkImpl::AddBook(const int year, const std::string& title,  const std::string& aid, std::vector<std::string>& tags){
    BookRepositoryImpl repo;
    auto work = StartTrasaction();
    repo.SetWork(&work);
    domain::AuthorId author_id{util::detail::UUIDFromString(aid)};
    repo.Save(domain::Book(domain::BookId::New(), author_id, title, year, tags));
    Commit(work);
}

std::vector<domain::BookRepr> UnitOfWorkImpl::GetBooks(std::string_view param) {
    BookRepositoryImpl repo;
    pqxx::read_transaction rt(GetConnection());
    repo.SetReadTransaction(&rt);
    return repo.GetBooks(param);
}

std::vector<domain::BookRepr> UnitOfWorkImpl::GetAuthorBooks(const std::string& aid) {
    BookRepositoryImpl repo;
    pqxx::read_transaction rt(GetConnection());
    repo.SetReadTransaction(&rt);
    return repo.GetAuthorBooks(aid);
}

void UnitOfWorkImpl::DeleteBook(std::string_view bid){
    BookRepositoryImpl repo;
    auto work = StartTrasaction();
    repo.SetWork(&work);
    repo.DeleteBook(bid);
    Commit(work);
}

void UnitOfWorkImpl::EditBook(const domain::BookRepr& book){
    BookRepositoryImpl repo;
    auto work = StartTrasaction();
    repo.SetWork(&work);
    repo.EditBook(book);
    Commit(work);
}

}