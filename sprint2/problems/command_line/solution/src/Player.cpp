#include "Player.h"

namespace app {

void Player::Move(std::chrono::microseconds period){
    dog_->Move(period);
}

}