#include "Players.h"

namespace app{

    Player* Players::Add(Dog* dog_ptr, GameSession* session){
        players_.emplace_back(session, dog_ptr);
        return &players_.back();
    }

    // Player* Players::FindByDogIdAndMapId(std::uint64_t dog_id, std::uint64_t map_id) {
    //     if(player_index_.count({dog_id, map_id}) == 0){
    //         return nullptr;
    //     }
    //     return player_index_.at({dog_id, map_id});
    // }

}