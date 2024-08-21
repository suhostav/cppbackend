#include "GameApp.h"

namespace app {

const model::GameSession* GameApp::GetPlayerSession(const Token token) const{
    Player* player = player_tokens_.FindPlayerByToken(token);
    if(player_session_.count(player) == 0){
        return nullptr;
    }
    return player_session_.at(player);
}

JoinInfo GameApp::JoinGame(std::string_view dog_name, std::string_view map_id_str){
    model::JoinResult result = game_.JoinGame(dog_name, map_id_str);
    Player* new_player = players_.Add(result.dog, result.session);
    Token token = player_tokens_.Add(*new_player);
    player_session_[new_player] = result.session;
    return {new_player, result.session, token};
}

void GameApp::SetPlayerSpeed(Token token, model::GameSession* session, char dir){
    app::Player* player = FindPlayerByToken(token);
    auto s = session->GetMap()->GeSpeed();
    model::DogDir dog_dir = static_cast<model::DogDir>(dir);
    player->GetDog()->SetDir(dog_dir);
    switch(dog_dir){
        case model::DogDir::WEST:
            player->GetDog()->SetSpeed(-s, 0);
            break;
        case model::DogDir::EAST:
            player->GetDog()->SetSpeed(s, 0);
            break;
        case model::DogDir::NORTH:
            player->GetDog()->SetSpeed(0, -s);
            break;
        case model::DogDir::SOUTH:
            player->GetDog()->SetSpeed(0, s);
            break;
    }
}

}   //namespace app