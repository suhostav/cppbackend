#pragma once

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
    void Move(std::int64_t period);
private:
    GameSession* session_;
    Dog* dog_;
};
} //namespace app