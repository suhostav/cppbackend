#pragma once

#include <chrono>
#include "model.h"

namespace app {

using namespace model;

class Player{
public:
    Player(GameSession* session, Dog* dog)
        : dog_(dog)
        , session_(session) {
    }
    Dog* GetDog(){
        return dog_;
    }
    GameSession* GetSession(){
        return session_;
    }
    void Move(std::chrono::milliseconds period);
    bool IsRetire() const {
        return dog_->IsRetire();
    }
    bool operator==(const Player& other) {
        return dog_ == other.dog_;
    }
private:
    GameSession* session_;
    Dog* dog_;
};
} //namespace app