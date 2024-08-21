#include "model.h"

#include <stdexcept>

namespace model {
using namespace std::literals;

    Map::Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Map::Id& Map::GetId() const noexcept {
        return id_;
    }

    const std::string& Map::GetName() const noexcept {
        return name_;
    }

    const Map::Buildings& Map::GetBuildings() const noexcept {
        return buildings_;
    }

    const Map::Roads& Map::GetRoads() const noexcept {
        return roads_;
    }

    const Map::Offices& Map::GetOffices() const noexcept {
        return offices_;
    }

    void Map::AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void Map::AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

void Map::AddOffice(Office office) {
    if (warehouse_id_to_index_.contains(office.GetId())) {
        throw std::invalid_argument("Duplicate warehouse");
    }

    const size_t index = offices_.size();
    Office& o = offices_.emplace_back(std::move(office));
    try {
        warehouse_id_to_index_.emplace(o.GetId(), index);
    } catch (...) {
        // Удаляем офис из вектора, если не удалось вставить в unordered_map
        offices_.pop_back();
        throw;
    }
}

GameSession::GameSession(model::Map* map): map_(map) {
}

Dog* GameSession::AddDog(std::string_view dog_name){
    Dog& new_dog = dogs_.emplace_back(dog_name);
    dogs_by_Ids_[new_dog.GetId()] = &new_dog;
    return &new_dog;
}

model::Map* GameSession::GetMap(){
    return map_;
}

Dog* GameSession::GetDogById(std::uint64_t id){
    if(dogs_by_Ids_.count(id) == 0){
        return nullptr;
    }
    return dogs_by_Ids_.at(id);
}

size_t GameSession::GetDogsNumber() const{
    return dogs_.size();
}

void Game::AddMap(Map map) {
    const size_t index = maps_.size();
    if (auto [it, inserted] = map_id_to_index_.emplace(map.GetId(), index); !inserted) {
        throw std::invalid_argument("Map with id "s + *map.GetId() + " already exists"s);
    } else {
        try {
            maps_.emplace_back(std::move(map));
        } catch (...) {
            map_id_to_index_.erase(it);
            throw;
        }
    }
}

JoinResult Game::JoinGame(std::string_view dog_name, std::string_view map_id_str){
    Map::Id map_id{std::string(map_id_str)};
    if(map_id_to_index_.count(map_id) == 0){
        throw BadMapIdException("Map not found"s);
    }
    size_t map_index = map_id_to_index_[map_id];
    std::vector<GameSession>& map_sessions = maps_sessions_[map_index];
    GameSession* join_session = nullptr;
    for(auto& session : map_sessions){
        if(session.GetDogsNumber() < max_map_dogs_){
            join_session = &session;
            break;
        }
    }
    if(!join_session){
        join_session = &maps_sessions_[map_index].emplace_back(&maps_[map_index]);
    }
    return {join_session->AddDog(std::string(dog_name)), join_session};
}

}  // namespace model

