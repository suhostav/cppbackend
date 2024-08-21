#include "sdk.h"
//
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <thread>

#include "logger.h"
#include "json_loader.h"
#include "GameApp.h"
#include "request_handler.h"
#include "Ticker.h"

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

struct Args {
    int tick_period = 0;
    std::string config;
    std::string root_dir;
    bool random_pos;
};

[[nodiscard]] std::optional<Args> ParseCommandLine(int argc, const char* const argv[]) {
    namespace po = boost::program_options;
    po::options_description desc{"All options"s};

    Args args;
    desc.add_options()
        ("help,h", "produce help message")
        ("tick-period,t", po::value(&args.tick_period)->value_name("miliseconds"s), "set tick period")
        ("config-file,c", po::value(&args.config)->value_name("file"s), "set config file path")
        ("www-root,w", po::value(&args.root_dir)->value_name("dir"s), "set static files root")
        ("randomize-spawn-points", po::value(&args.random_pos), "spawn dogs at random positions");
    
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    std::optional<Args> result{args};

    if (vm.contains("help"s)) {
        std::cout << desc;
        return std::nullopt;
    }

    // Проверяем наличие опций src и dst
    if (!vm.contains("config-file"s)) {
        std::cout << "Config-file file path is not specified\n"s;
        result = std::nullopt;
    }
    if (!vm.contains("www-root"s)) {
        std::cout << "static files root path is not specified\n"s;
        result = std::nullopt;
    }
    result->random_pos = vm.contains("randomize-spawn-points");
    if(!result){
        std::cout << "Usage:\n";
        std::cout << desc;
    }

    return result;
}

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)

int main(int argc, const char* argv[]) {
    InitLog();
    Args args;
    if(argc == 3){
        args.config = "data/config.json";
        args.root_dir = "static";
        args.tick_period = 0;
        args.random_pos = false;
    } else {
        try{
            auto args_opt = ParseCommandLine(argc, argv);
            if(!args_opt){
                return EXIT_FAILURE;
            }
            args = *args_opt;
        } catch(const std::exception& e){
            std::cout << e.what() << std::endl;
            return EXIT_FAILURE;
        }
    }
    // std::string static_files_root{};
    // std::filesystem::path conf_path;
    //===================================================
    // if (argc != 3) {
    //     std::filesystem::path prog{argv[0]};
    //     prog.remove_filename();
    //     auto static_path = prog / "../../static";
    //     prog/= "../../data/config.json";
    //     conf_path = prog.lexically_normal();
    //     static_files_root = static_path.lexically_normal().string();
    // } else {
    //     static_files_root = argv[2];
    //     conf_path = argv[1];
    // }


    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(args.config);
        game.SetRandomPoint(args.random_pos);
        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);
        auto api_strand = net::make_strand(ioc);
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
        app::GameApp game_app(game);
        auto api_handler = std::make_shared<http_handler::ApiHandler>(game_app, args.tick_period == 0);
        auto handler = std::make_shared<http_handler::RequestHandler>(
            args.root_dir, api_strand, api_handler
        );
        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, handler);
        

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        // std::cout << "Server has started..."sv << std::endl;df
        boost::json::value start_data{{"port"s, (int)port}, {"address"s, address.to_string()}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, start_data) << "server started"sv;

        //запускаем движение
        if(args.tick_period > 0){
            auto ticker = std::make_shared<Ticker>(api_strand, args.tick_period * 1ms, [&game_app](std::chrono::milliseconds delta){
                game_app.Move(delta);
            });
            ticker->Start();
        }

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

