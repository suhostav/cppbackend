#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <thread>

#include "tagged.h"
#include "logger.h"
#include "json_loader.h"
#include "request_handler.h"
#include "PlayerTokens.h"

using namespace std::literals;
namespace net = boost::asio;
namespace sys = boost::system;

namespace {

// Запускает функцию fn на n потоках, включая текущий
template <typename Fn>
void RunWorkers(unsigned n, const Fn& fn) {
    n = std::max(1u, n);
    std::vector<std::jthread> workers;
    workers.reserve(n - 1);
    // Запускаем n-1 рабочих потоков, выполняющих функцию fn
    while (--n) {
        workers.emplace_back(fn);
    }
    fn();
}

}  // namespace

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)

int main(int argc, const char* argv[]) {
    InitLog();
    PlayerTokens tk;
    auto token = *tk.GetToket();
    std::cout << token << ", lenght = " << token.size() << std::endl;
    if (argc != 3) {
        std::cerr << "Usage: game_server <game-config-json> <static_dir>"sv << std::endl;
        return EXIT_FAILURE;
    }
    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(argv[1]);
        std::string static_dir{argv[2]};
        std::string base_dir{static_dir};
        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);

        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
            if (!ec) {
                ioc.stop();
                boost::json::value exit_data{{"code"s, EXIT_SUCCESS}};
                BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, exit_data) << "server exited"sv;
            } else {
                boost::json::value exit_data{{"code"s, EXIT_FAILURE}};
                BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, exit_data) << "server exited"sv;
            }
        });

        // 4. Создаём обработчик HTTP-запросов и связываем его с моделью игры
        http_handler::RequestHandler handler(static_dir, net::make_strand(ioc), game);
        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, base_dir, 
            [&handler](auto&& req, auto&& send, const std::string& base_dir) {
            handler(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
        });
        

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        // std::cout << "Server has started..."sv << std::endl;df
        boost::json::value start_data{{"port"s, (int)port}, {"address"s, address.to_string()}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, start_data) << "server started"sv;

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });
    } catch (const std::exception& ex) {
        boost::json::value exit_data{{"code"s, EXIT_FAILURE},
            {"exception"s, ex.what()}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, exit_data) << "server exited"sv;
        return EXIT_FAILURE;
    }

}

