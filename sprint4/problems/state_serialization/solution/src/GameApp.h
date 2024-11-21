#pragma once

#include <chrono>
#include <unordered_map>
#include <set>
#include "model.h"
#include "Player.h"
#include "PlayerTokens.h"
#include "model_serialization.h"


namespace app{


struct JoinInfo{
    Player* player;
    model::GameSession* session;
    Token token;
};

class GameApp {
public:
    using Milliseconds = std::chrono::milliseconds;
    explicit GameApp(model::Game& game, std::string save_file, Milliseconds save_period)
        : game_(game)
        , save_file_(save_file)
        , time_from_save_(0)
        , save_period_(save_period){
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
    void Move(Milliseconds period);
    bool PeriodicSave(Milliseconds delta, std::string& err_msg);
    bool Save(std::string& err_msg);
    bool Restore(std::string& err_msg);

private:
    void RestoreSession(serialization::SessionRepr& session_repr, std::vector<serialization::TokenRepr>& tokens);
    model::Game& game_;
    Players players_;
    PlayerTokens player_tokens_;
    std::set<model::GameSession*> sessions_;
    std::unordered_map<Player*, model::GameSession*> player_session_;
    std::string save_file_;
    Milliseconds time_from_save_;
    Milliseconds save_period_;
    
};
}