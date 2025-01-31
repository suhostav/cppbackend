#pragma once

#include <random>
#include <string>
#include <sstream>
#include <unordered_map>

#include "tagged.h"
#include "Player.h"
#include "Players.h"

namespace detail {
struct TokenTag {};
}  // namespace detail

namespace app {
using Token = util::Tagged<std::string, detail::TokenTag>;
// class Player;

class PlayerTokens {
public:
    PlayerTokens() = default;
    Token GetToken();
    Token Add(Player& player);
    void Add(Player* player, Token& token);
    Player* FindPlayerByToken(Token token) const;
    std::vector<std::pair<std::uint64_t, std::string>> GetAllTokens() const;
    void RemovePlayerToken(Player* player);
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
}   //namespace app