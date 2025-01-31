#pragma once 

#include <pqxx/connection>
#include <pqxx/transaction>
#include <memory>
#include <condition_variable>



class ConnectionPool{
    using PoolType = ConnectionPool;
    using ConnectionPtr = std::shared_ptr<pqxx::connection>;
public:
    class ConnectionWrapper {
    public:
        ConnectionWrapper(std::shared_ptr<pqxx::connection>&& conn, PoolType& pool) noexcept 
            : conn_{std::move(conn)} 
            , pool_{&pool} {
        }
        ConnectionWrapper(const ConnectionWrapper&) = delete;
        ConnectionWrapper& operator=(const ConnectionWrapper&) = delete;
        pqxx::connection& operator*() const& noexcept { return *conn_; }
        pqxx::connection& operator*() const&& = delete;
        pqxx::connection* operator->() const& noexcept { return conn_.get(); }
        ~ConnectionWrapper() {
            if(conn_) {
                pool_->ReturnConnection(std::move(conn_));
            }
        }
    private:
        std::shared_ptr<pqxx::connection> conn_;
        PoolType* pool_;
    };

    template <typename ConnectionFactory>
    ConnectionPool(size_t capacity, ConnectionFactory&& factory){
        pool_.reserve(capacity);
        for(size_t i = 0; i < capacity; ++i){
            pool_.emplace_back(factory());
        }
    }

    bool HasFreeConnection(){
        return used_connections_ < pool_.size();
    }
    
    ConnectionWrapper GetConnection(){
        std::unique_lock lock{mutex_};
        cond_var_.wait(lock, [this]{
            return this->HasFreeConnection();
        });

        return {std::move(pool_[used_connections_++]), *this};
    }
private:
    void ReturnConnection(ConnectionPtr&& conn){
        std::lock_guard lock{mutex_};
        assert(used_connections_ != 0);
        pool_[--used_connections_] = std::move(conn);
    }
    std::mutex mutex_;
    std::condition_variable cond_var_;
    std::vector<ConnectionPtr> pool_;
    size_t used_connections_ = 0;
};