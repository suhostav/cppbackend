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
    DogPoint p{0, 0};
    // choose road
    size_t n_roads = map_->GetRoads().size();
    // std::uniform_real_distribution<std::mt19937_64::result_type> dist_roads(0, n_roads - 1);
    std::uniform_int_distribution<std::mt19937_64::result_type> dist_roads(0, n_roads - 1);
    size_t road_index = dist_roads(random_device_);
    const Road& road = map_->GetRoads()[road_index];
    // choose point on road
    Coord length = road.GetLength();
    // length = length > 0 ? length - 1 : length + 1;
    std::uniform_int_distribution<std::mt19937_64::result_type> dist_length(0, length * 100);
    DogCoord road_shift = dist_length(random_device_) / 100.;
    DogPoint road_point = road.IsVertical() ? DogPoint{(double)road.GetStart().x, (double)road.GetStart().y + road_shift}
        : DogPoint{(double)road.GetStart().x + road_shift, (double)road.GetStart().y};
    Dog& new_dog = dogs_.emplace_back(dog_name, road_point);
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
    std::deque<GameSession>& map_sessions = maps_sessions_[map_index];
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

