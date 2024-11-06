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

Player* PlayerTokens::FindPlayerByToken(Token token) const{
    if(token_to_player_.count(token) == 0){
        return nullptr;
    }
    return token_to_player_.at(token);
}

}