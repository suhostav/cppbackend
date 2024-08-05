#pragma once
#include <cstdint>
#include <string>

class Dog {
public:
    Dog(std::string& name)
        : name_(std::move(name))
        , id_{next_id_++} {
    }
    std::uint32_t GetId(){return id_;}
private:
    std::string name_;
    std::uint32_t id_;
    static std::uint32_t next_id_;
};
