#pragma once

#include <chrono>
#include <unordered_map>
#include "model.h"
#include "Player.h"
#include "PlayerTokens.h"


namespace app{


struct JoinInfo{
    Player* player;
    model::GameSession* session;
    Token token;
};

class GameApp {
public:
    GameApp(model::Game& game): game_(game){
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        return game_.FindMap(id);
    }

    const model::Game::Maps& GetMaps() const noexcept {
        return game_.GetMaps();
    }

    Player* FindPlayerByToken(Token token) const{
        return player_tokens_.FindPlayerByToken(token);
    }

    const model::GameSession* GetPlayerSession(const Token token) const;

    JoinInfo JoinGame(std::string_view dog_name, std::string_view map_id_str);
    void SetPlayerSpeed(Token token, model::GameSession* session, char dir);
    void Move(std::chrono::microseconds period);

private:
    model::Game& game_;
    Players players_;
    PlayerTokens player_tokens_;
    std::unordered_map<Player*, model::GameSession*> player_session_;

};
}