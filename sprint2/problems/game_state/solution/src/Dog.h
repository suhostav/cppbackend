#pragma once
#include <cstdint>
#include <string>

namespace model{

using DogCoord = double;


struct DogPoint {
    DogCoord h; //horizontal coord
    DogCoord v; //vertical coord
};

using DogSpeedPerSecond = double;

struct DogSpeed {
    DogSpeed(DogSpeedPerSecond h, DogSpeedPerSecond v): hs(h), vs(v){}
    DogSpeedPerSecond hs;    //horizontal speed
    DogSpeedPerSecond vs;    //vertical speed
};

enum class DogDir {
    NORTH = 'U',
    SOUTH = 'S',
    WEST = 'L',
    EAST = 'R'
};

class Dog {
public:
    Dog(std::string_view name, DogPoint p)
        : name_(name.begin(), name.end())
        , point_(p)
        , id_{next_id_++} {
    }
    std::uint64_t GetId(){return id_;}
    const std::string& GetName() const {
        return name_;
    }

    const DogPoint& GetPoint() const {
        return point_;
    }

    const DogSpeed& GetSpeed() const {
        return speed_;
    }

    const DogDir GetDir() const {
        return dir_;
    }
    
private:
    std::string name_;
    std::uint64_t id_;
    DogPoint point_;
    DogSpeed speed_ = {0.0, 0.0};
    DogDir dir_ = {DogDir::NORTH};

    static std::uint64_t next_id_;
};
}   //namespace model