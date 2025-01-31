#pragma once

#include <chrono>
#include <unordered_map>
#include <deque>
#include <vector>
#include "Player.h"

namespace app {

using namespace model;

class Players{
public:
    Players() = default;

    Player* Add(Dog* dog_ptr, GameSession* session);
    void Move(std::chrono::milliseconds period);
    std::deque<Player>& GetPlayers() {
        return players_;
    }
private:
    std::deque<Player> players_;
};
}   //namespace app