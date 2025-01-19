#pragma once
#include <pqxx/pqxx>

#include "app/use_cases_impl.h"
#include "app/UntiOfWork.h"
#include "postgres/postgres.h"
#include "postgres/UnitOfWorkImpl.h"

namespace bookypedia {

struct AppConfig {
    std::string db_url;
};

class Application {
public:
    explicit Application(app::UnitOfWorkFactory& factory);

    void Run();

private:
    // postgres::Database db_;
    app::UseCasesImpl use_cases_;

};

}  // namespace bookypedia
