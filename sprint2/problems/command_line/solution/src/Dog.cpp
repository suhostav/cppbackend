#include "Dog.h"

namespace model {
std::uint64_t Dog::next_id_ = 0;

void Dog::Move(std::chrono::microseconds period){
    double seconds = (double)period.count() / 1000.;
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

}