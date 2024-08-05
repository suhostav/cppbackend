#pragma once

#include <random>
#include <string>
#include <sstream>
#include <unordered_map>

#include "tagged.h"
#include "Player.h"

namespace detail {
struct TokenTag {};
}  // namespace detail

using Token = util::Tagged<std::string, detail::TokenTag>;

class PlayerTokens {
public:
    PlayerTokens() = default;
    Token GetToket(){
        std::stringstream ss;
        ss << std::hex << generator1_() << generator2_();
        auto result = ss.str();
        if(result.size() < 32){
            result = std::string(32 - result.size(), '0') + result;
        }
        return Token{result};
    }

    Token Add(Player& player){
        Token token = GetToket();
        token_to_player_[token] = &player;
        return token;
    }
    Player* FindPlayerByToken(Token token) const{
        if(token_to_player_.count(token) == 0){
            return nullptr;
        }
        return token_to_player_.at(token);
    }
private:
    std::random_device random_device_;
    std::mt19937_64 generator1_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::mt19937_64 generator2_{[this] {
        std::uniform_int_distribution<std::mt19937_64::result_type> dist;
        return dist(random_device_);
    }()};
    std::unordered_map<Token, Player*, util::TaggedHasher<Token>> token_to_player_;
};