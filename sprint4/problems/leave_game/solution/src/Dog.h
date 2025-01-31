#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>
#include "coords.h"
#include "geom.h"
#include "loot.h"

namespace model{

using namespace std::chrono;
using namespace std::chrono_literals;

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

    Dog(std::string_view name, geom::Point2D p, size_t bag_capacity, std::chrono::seconds stop_duration)
        : name_(name.begin(), name.end())
        , point_(p)
        , prev_point_(p)
        , id_{next_id_++}
        , bag_capacity_{bag_capacity}
        , stop_duration_(stop_duration) {
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
        stop_period_ = 0s;
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

    bool IsActive() const {
        return (speed_.hs != 0.0) || (speed_.vs != 0);
    }
    const std::chrono::milliseconds& GetTotalTime() const {
        return total_time_;
    }
    int GetTotalSeconds() const {
        return duration_cast<std::chrono::seconds>(total_time_).count();
    }
    void SetStopPeriod(std::chrono::milliseconds stop_period){
        stop_period_ = stop_period;
    }
    const std::chrono::milliseconds& GetStopPeriod() const {
        return stop_period_;
    }
    void SetStopDuration(std::chrono::seconds stop_duration) {
        stop_duration_ = stop_duration;
    }
    const std::chrono::seconds& GetStopDuration() const {
        return stop_duration_;
    }
    bool IsRetire() const {
        return duration_cast<std::chrono::seconds>(stop_period_) >= stop_duration_;
    }

    bool operator==(const Dog& other) {
        return id_ == other.GetId();
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
    std::chrono::milliseconds total_time_ = 0s;
    std::chrono::milliseconds stop_period_ = 0s;
    std::chrono::seconds stop_duration_ = 10s;
};

}   //namespace model