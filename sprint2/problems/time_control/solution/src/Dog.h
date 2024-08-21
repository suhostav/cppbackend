#pragma once
#include <cstdint>
#include <string>
#include "coords.h"

namespace model{

class Dog {
public:
    Dog(std::string_view name, DPoint p)
        : name_(name.begin(), name.end())
        , point_(p)
        , id_{next_id_++} {
    }
    std::uint64_t GetId(){return id_;}
    const std::string& GetName() const {
        return name_;
    }

    const DPoint& GetPoint() const {
        return point_;
    }

    const DogSpeed& GetSpeed() const {
        return speed_;
    }

    const DogDir GetDir() const {
        return dir_;
    }

    void SetSpeed(DogSpeedPerSecond hs, DogSpeedPerSecond vs){
        speed_.hs = hs;
        speed_.vs = vs;
    }

    void SetDir(DogDir dir, DCoord limit){
        dir_ = dir;
        limit_ = limit;
    }

    void Stop(){
        speed_ = {0.0, 0.0};
    }

    void Move(std::int64_t period);
    
private:
    std::string name_;
    std::uint64_t id_;
    DPoint point_;
    DogSpeed speed_ = {0.0, 0.0};
    DogDir dir_ = {DogDir::NORTH};
    DCoord limit_ = 0.0;

    static std::uint64_t next_id_;
};
}   //namespace model