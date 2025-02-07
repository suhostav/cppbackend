#include "Dog.h"

namespace model {
std::uint64_t Dog::next_id_ = 0;

void Dog::Move(std::chrono::milliseconds period){

    total_time_ += period;

    if(speed_.hs == 0.0 && speed_.vs == 0.0){
        stop_period_ += period;
        if(IsRetire()) {
            return;
        }
    }
    double seconds = static_cast<double>(period.count()) / 1000.;
    prev_point_ = point_;
    switch (dir_)
    {
    case DogDir::NORTH:
        point_.y += speed_.vs * seconds;
        if(point_.y < limit_){
            point_.y = limit_;
            Stop();
        }
        break;
    case DogDir::SOUTH:
        point_.y += speed_.vs * seconds;
        if(point_.y > limit_){
            point_.y = limit_;
            Stop();
        }
        break;
    case DogDir::WEST:
        point_.x += speed_.hs * seconds;
        if(point_.x < limit_){
            point_.x = limit_;
            Stop();
        }
        break;
    case DogDir::EAST:
        point_.x += speed_.hs * seconds;
        if(point_.x > limit_){
            point_.x = limit_;
            Stop();
        }
        break;

    default:
        break;
    }
}

bool Dog::AddLoot(const Loot& loot){
    if(loots_.size() >= bag_capacity_ || loot.taken){
        return false;
    }
    loots_.push_back(loot);
    return true;
}

void Dog::DropLoots(){
    loots_.clear();
}

}