#include "Player.h"

namespace app {

void Player::Move(std::int64_t period){
    dog_->Move(period);
}

}