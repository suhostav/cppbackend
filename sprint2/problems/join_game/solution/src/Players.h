#pragma once

#include <unordered_map>
#include <vector>
#include "Player.h"

class Players{
public:
    Players() = default;

    Player& Add(Dog dog, GameSession& session){
        Dog* dog_ptr = session.AddDog(std::move(dog));
        players_.emplace_back(dog_ptr, session.GetMap());
        return players_.back();
    }

    Player* FindByDogIdAndMapId(std::uint64_t dog_id, std::uint64_t map_id) {
        if(player_index_.count({dog_id, map_id}) == 0){
            return nullptr;
        }
        return player_index_.at({dog_id, map_id});
    }
private:
    std::vector<Player> players_;
    std::unordered_map<std::pair<std::uint64_t,std::uint64_t>, Player*> player_index_;
};