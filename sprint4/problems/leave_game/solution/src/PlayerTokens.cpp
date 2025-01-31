#include "PlayerTokens.h"

namespace app {

Token PlayerTokens::GetToken(){
    std::stringstream ss;
    ss << std::hex << generator1_() << generator2_();
    auto result = ss.str();
    if(result.size() < 32){
        result = std::string(32 - result.size(), '0') + result;
    }
    return Token{result};
}

Token PlayerTokens::Add(Player& player){
    Token token = GetToken();
    token_to_player_[token] = &player;
    return token;
}

void PlayerTokens::Add(Player* player, Token& token){
    token_to_player_[token] = player;
}

Player* PlayerTokens::FindPlayerByToken(Token token) const{
    if(token_to_player_.count(token) == 0){
        return nullptr;
    }
    return token_to_player_.at(token);
}

void PlayerTokens::RemovePlayerToken(Player* player) {
    for( auto [next_token, next_player] : token_to_player_){
        if( next_player == player){
            token_to_player_.erase(next_token);
            return;
        }
    }
}

std::vector<std::pair<std::uint64_t, std::string>> PlayerTokens::GetAllTokens() const{
    std::vector<std::pair<std::uint64_t, std::string>> tokens_players;
    for(const auto [token,player] : token_to_player_){
        tokens_players.push_back({player->GetDog()->GetId(), *token});
    }
    return tokens_players;
}
}