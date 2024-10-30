#include "Player.h"

namespace app {

void Player::Move(std::chrono::milliseconds period){
    dog_->Move(period);
}

}