#pragma once

#include <unordered_map>
#include <vector>
#include "Player.h"

namespace app {

using namespace model;

class Players{
public:
    Players() = default;

    Player* Add(Dog* dog_ptr, GameSession* session);
    void Move(std::int64_t period);
private:
    std::vector<Player> players_;
};
}   //namespace app