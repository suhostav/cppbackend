#pragma once
#include <optional>
#include <string>
#include <vector>
#include "../util/tagged_uuid.h"

namespace domain {

namespace detail {
struct AuthorTag {};
}  // namespace detail

using AuthorId = util::TaggedUUID<detail::AuthorTag>;

class Author {
public:
    Author(AuthorId id, std::string name)
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const AuthorId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

private:
    AuthorId id_;
    std::string name_;
};

class AuthorRepository {
public:
    virtual std::string Save(const Author& author) = 0;
    virtual bool Delete(const std::string& aid_str) = 0;
    virtual std::vector<Author> GetAuthors() = 0;
    virtual std::optional<Author> GetAuthorByName(const std::string_view name) = 0;
    virtual void RenameAuthor(std::string_view aid_str, std::string_view name) = 0;
protected:
    ~AuthorRepository() = default;
};

}  // namespace domain
