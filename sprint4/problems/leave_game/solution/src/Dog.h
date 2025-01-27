#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include "coords.h"
#include "geom.h"
#include "loot.h"

namespace model{

class Dog {
public:
    using BagContent = std::vector<Loot>;
    Dog(std::uint64_t id, std::string_view name, geom::Point2D p, DogSpeed speed, size_t bag_capacity)
        : id_(id)
        , name_(name.begin(), name.end())
        , point_(p)
        , prev_point_(p)
        , speed_(speed)
        , bag_capacity_{bag_capacity} {
    }

    Dog(std::string_view name, geom::Point2D p, size_t bag_capacity)
        : name_(name.begin(), name.end())
        , point_(p)
        , prev_point_(p)
        , id_{next_id_++}
        , bag_capacity_{bag_capacity} {
    }

    std::uint64_t GetId() const {return id_;}

    const std::string& GetName() const {
        return name_;
    }

    geom::Point2D GetPoint2D() const {
        return {point_.x, point_.y};
    }

    geom::Point2D GetPrevPoint2D() const {
        return prev_point_;
    }

    const DogSpeed& GetSpeed() const {
        return speed_;
    }

    const DogDir GetDir() const {
        return dir_;
    }

    const DCoord GetLimit() const {
        return limit_;
    }
    void SetSpeed(DogSpeedPerSecond hs, DogSpeedPerSecond vs){
        speed_.hs = hs;
        speed_.vs = vs;
    }

    void SetSpeed(DogSpeed speed){
        speed_ = speed;
    }

    void SetDir(DogDir dir, DCoord limit){
        dir_ = dir;
        limit_ = limit;
    }

    void Stop(){
        speed_ = {0.0, 0.0};
    }

    void Move(std::chrono::milliseconds period);
    bool AddLoot(const Loot& loot);
    void DropLoots();
    const std::vector<Loot>& GetLoots() const{
        return loots_;
    }

    void AddScore(int value){
        score_ += value;
    }

    void SetScore(int value){
        score_ = value;
    }

    int GetScore() const {
        return score_;
    }

    size_t GetBagCapacity() const {
        return bag_capacity_;
    }

    BagContent GetBagContent() const {
        return loots_;
    }

    static std::uint64_t next_id_;

private:
    std::uint64_t id_;
    std::string name_;
    geom::Point2D point_;
    geom::Point2D prev_point_;
    DogSpeed speed_ = {0.0, 0.0};
    DogDir dir_ = {DogDir::NORTH};
    DCoord limit_ = 0.0;
    size_t bag_capacity_;
    BagContent loots_;
    int score_ = 0;

};
}   //namespace model