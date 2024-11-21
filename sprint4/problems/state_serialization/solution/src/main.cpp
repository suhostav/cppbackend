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
    std::string save_file;
    int save_period;
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
        ("randomize-spawn-points,r", po::value(&args.random_pos), "spawn dogs at random positions")
        ("state-file,s", po::value(&args.save_file)->value_name("file"s), "set save state file path")
        ("save-state-period,p", po::value(&args.save_period)->value_name("miliseconds"s), "set save state period");
    
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
    if (!vm.contains("state-file"s)) {
        std::cout << "state-file file path is not specified\n"s;
        result = std::nullopt;
    }

    if (!vm.contains("randomize-spawn-points"s)) {
        result->random_pos = false;
    } else {
        result->random_pos = true;
    }
    return result;
}

BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)

int main(int argc, const char* argv[]) {
    InitLog();
    Args args{.tick_period = 0, .random_pos = false, .save_file = "", .save_period = 0};
    if(argc == 1){
        args.config = "../../data/config.json";
        args.root_dir = "../../static";
        args.tick_period = 200;
        args.save_file = "../../data/saved_data.txt";
        args.save_period = 3000;
    } else if(argc == 3){
        args.config = "data/config.json";
        args.root_dir = "static";
        args.save_file = "data/saved_data.txt";
        args.save_period = 3000;
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

    try {
        // 1. Загружаем карту из файла и построить модель игры
        model::Game game = json_loader::LoadGame(args.config);
        std::cout << "Random point: " <<(args.random_pos?"true":"false") << std::endl;
        game.SetRandomPoint(args.random_pos);
        // 2. Инициализируем io_context
        const unsigned num_threads = std::thread::hardware_concurrency();
        net::io_context ioc(num_threads);
        auto api_strand = net::make_strand(ioc);
        app::GameApp game_app(game, args.save_file, args.save_period * 1ms);
        if(std::filesystem::exists(args.save_file)){
            std::string msg;
            game_app.Restore(msg);
            if(!msg.empty()){
                boost::json::value exit_data{{"code"s, EXIT_FAILURE}};
                BOOST_LOG_TRIVIAL(info) << logging::add_value(msg, exit_data) << "server exited"sv;
                return EXIT_FAILURE;
            }
        }
        // 3. Добавляем асинхронный обработчик сигналов SIGINT и SIGTERM
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, &args](const sys::error_code& ec, [[maybe_unused]] int signal_number) {
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
        auto api_handler = std::make_shared<http_handler::ApiHandler>(game_app, args.tick_period == 0);
        auto handler = std::make_shared<http_handler::RequestHandler>(
            args.root_dir, api_strand, api_handler
        );
        // 5. Запустить обработчик HTTP-запросов, делегируя их обработчику запросов
        
        const auto address = net::ip::make_address("0.0.0.0");
        constexpr net::ip::port_type port = 8080;
        http_server::ServeHttp(ioc, {address, port}, handler);
        

        // Эта надпись сообщает тестам о том, что сервер запущен и готов обрабатывать запросы
        boost::json::value start_data{{"port"s, static_cast<int>(port)}, {"address"s, address.to_string()}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, start_data) << "server started"sv;

        //запускаем движение
        if(args.tick_period > 0){
            // std::cout << "tick period: " << args.tick_period << std::endl;
            auto ticker = std::make_shared<Ticker>(api_strand, args.tick_period * 1ms, [&game_app](std::chrono::milliseconds delta){
                game_app.Move(delta);
            });
            ticker->Start();
        }

        // 6. Запускаем обработку асинхронных операций
        RunWorkers(std::max(1u, num_threads), [&ioc] {
            ioc.run();
        });

        //7. Сохраняем результаты
        if(args.save_file.size() > 0){
            std::string msg;
            bool saved = game_app.Save(msg);
            if(!saved){
                boost::json::value exit_data{msg, EXIT_FAILURE};
                BOOST_LOG_TRIVIAL(error) << logging::add_value(additional_data, exit_data) << "server exited"sv;
                return EXIT_FAILURE;
            } 
        }
    } catch (const std::exception& ex) {
        boost::json::value exit_data{{"code"s, EXIT_FAILURE},
            {"exception"s, ex.what()}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, exit_data) << "server exited"sv;
        return EXIT_FAILURE;
    }
}