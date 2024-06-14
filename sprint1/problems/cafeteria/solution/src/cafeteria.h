#pragma once
#ifdef _WIN32
#include <sdkddkver.h>
#endif

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/bind_executor.hpp>
#include <chrono>
#include <memory>
#include <sstream>

#include "hotdog.h"
#include "result.h"

namespace net = boost::asio;
using namespace std::chrono;
using namespace std::literals;
using Timer = net::steady_timer;

// Функция-обработчик операции приготовления хот-дога
using HotDogHandler = std::function<void(Result<HotDog> hot_dog)>;

class Logger {
public:
    explicit Logger(std::string id)
        : id_(std::move(id)) {
    }

    void LogMessage(std::string_view message) const {
        std::osyncstream os{std::cout};
        os << id_ << "> ["sv << duration<double>(steady_clock::now() - start_time_).count()
           << "s] "sv << message << std::endl;
    }

private:
    std::string id_;
    steady_clock::time_point start_time_{steady_clock::now()};
};

class Order : public std::enable_shared_from_this<Order> {
public:

    Order(net::io_context& io, int id, 
        std::shared_ptr<Sausage> sausage, 
        std::shared_ptr<Bread> bread, 
        std::shared_ptr<GasCooker> gas_cooker,
        HotDogHandler handler )
        : io_{io}
        , id_{id}
        , sausage_(std::move(sausage))
        , bread_(std::move(bread))
        , gas_cooker_(std::move(gas_cooker))
        , handler_{std::move(handler)} {
    }

    // Запускает асинхронное выполнение заказа
    void Execute() {
        try{
            BakeBread();
            FrySausage();
            // net::post(io_, [self = shared_from_this()]{
            //     self->BakeBread();
            // });
            // net::post(io_, [self = shared_from_this()]{
            //     self->FrySausage();
            // });
        } catch(...){
            return handler_(std::current_exception());
        }
    }
private:
    net::io_context& io_;
    net::strand<net::io_context::executor_type> strand_{net::make_strand(io_)};
    int id_;
    Logger logger_{std::to_string(id_)};
    Timer sausage_timer_{io_, HotDog::MIN_SAUSAGE_COOK_DURATION};
    Timer bread_timer_{io_, HotDog::MIN_BREAD_COOK_DURATION};
    std::shared_ptr<Sausage> sausage_;
    std::shared_ptr<Bread> bread_;
    std::shared_ptr<GasCooker> gas_cooker_;
    HotDogHandler handler_;
    bool sausage_prepared_ = false;
    bool bread_prepared_ = false;
    
    void BakeBread(){
        bread_->StartBake(*gas_cooker_, [self = shared_from_this()]{
            self->bread_timer_.expires_after(HotDog::MIN_BREAD_COOK_DURATION);
            self->bread_timer_.async_wait([self2 = self->shared_from_this()](sys::error_code ec){
                self2->bread_->StopBaking();
                self2->OnBreadBaked(ec);
            });
        });
    }

    void OnBreadBaked(sys::error_code ec){
        if(ec){
            logger_.LogMessage("Bake error : "s + ec.what());
        } 
        bread_prepared_ = true;
        CheckReadiness(ec);
    }

    void FrySausage() {
        sausage_->StartFry(*gas_cooker_, [self = shared_from_this()]{
            self->logger_.LogMessage("Start frying sausage"sv);
            self->sausage_timer_.expires_after(HotDog::MIN_SAUSAGE_COOK_DURATION);
            self->sausage_timer_.async_wait([self2 = self->shared_from_this()](sys::error_code ec){
                self2->sausage_->StopFry();
                self2->OnSausageFring(ec);
            });
        });
    }

    void OnSausageFring(sys::error_code ec){
        if(ec){
            logger_.LogMessage("Marinade onion error: "s + ec.what());
        }
        sausage_prepared_ = true;
        CheckReadiness(ec);
    }

    void CheckReadiness(sys::error_code ec){
        if(ec){
            logger_.LogMessage("CheckReadiness: error"s + ec.what());
            return;
        }
        if(bread_prepared_ && sausage_prepared_){
            logger_.LogMessage("Ingradients prepared"sv);
            try{
                HotDog hotdog{id_, sausage_, bread_};
                handler_(hotdog);
            } catch(...){
                return handler_(std::current_exception());
            }
        }
    }

}; 


// Класс "Кафетерий". Готовит хот-доги
class Cafeteria {
public:
    explicit Cafeteria(net::io_context& io)
        : io_{io} {
    }

    // Асинхронно готовит хот-дог и вызывает handler, как только хот-дог будет готов.
    // Этот метод может быть вызван из произвольного потока
    void OrderHotDog(HotDogHandler handler) {
        auto bread = store_.GetBread();
        auto sausage = store_.GetSausage();
        int order_id = ++order_id_;
        std::make_shared<Order>(io_, order_id, sausage, bread, gas_cooker_, handler)->Execute();
    }

private:
    net::io_context& io_;
    // Используется для создания ингредиентов хот-дога
    Store store_;
    // Газовая плита. По условию задачи в кафетерии есть только одна газовая плита на 8 горелок
    // Используйте её для приготовления ингредиентов хот-дога.
    // Плита создаётся с помощью make_shared, так как GasCooker унаследован от
    // enable_shared_from_this.
    std::shared_ptr<GasCooker> gas_cooker_ = std::make_shared<GasCooker>(io_);
    int order_id_ = 0;
    Logger logger_{"Cafeteria"s};
};