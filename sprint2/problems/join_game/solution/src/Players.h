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
    // Player* FindByDogIdAndMapId(std::uint64_t dog_id, std::uint64_t map_id);
private:
    std::vector<Player> players_;
};
}   //namespace app