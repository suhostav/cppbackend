#pragma once
#include <cstdint>
#include <string>

namespace model{

class Dog {
public:
    Dog(std::string_view name)
        : name_(name.begin(), name.end())
        , id_{next_id_++} {
    }
    std::uint64_t GetId(){return id_;}
    const std::string& GetName() const {
        return name_;
    }
private:
    std::string name_;
    std::uint64_t id_;
    static std::uint64_t next_id_;
};
}   //namespace model