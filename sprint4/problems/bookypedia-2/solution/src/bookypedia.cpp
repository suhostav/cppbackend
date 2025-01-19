#include "bookypedia.h"

#include <iostream>

#include "menu/menu.h"
#include "postgres/postgres.h"
#include "ui/view.h"
#include "app/UntiOfWork.h"
#include "postgres/UnitOfWorkImpl.h"


namespace bookypedia {

using namespace std::literals;

app::UnitOfWorkFactory CreateUnitOfWork(const AppConfig& config){
    postgres::UnitOfWorkImpl uwork(config.db_url);
    return app::UnitOfWorkFactory(uwork);
}

Application::Application(app::UnitOfWorkFactory& factory)
    : use_cases_{factory} {
}

void Application::Run() {
    menu::Menu menu{std::cin, std::cout};
    menu.AddAction("Help"s, {}, "Show instructions"s, [&menu](std::istream&) {
        menu.ShowInstructions();
        return true;
    });
    menu.AddAction("Exit"s, {}, "Exit program"s, [&menu](std::istream&) {
        return false;
    });
    ui::View view{menu, use_cases_, std::cin, std::cout};
    menu.Run();
}

}  // namespace bookypedia
