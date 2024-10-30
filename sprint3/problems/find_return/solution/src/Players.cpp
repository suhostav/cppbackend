#include "Players.h"

namespace app{

    Player* Players::Add(Dog* dog_ptr, GameSession* session){
        players_.emplace_back(session, dog_ptr);
        return &players_.back();
    }

    void Players::Move(std::chrono::milliseconds period){
        for(auto& player : players_){
            player.Move(period);
        }
    }

}