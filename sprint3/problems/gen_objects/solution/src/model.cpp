#include "model.h"

#include <stdexcept>
#include <iostream>
namespace model {
using namespace std::literals;

// -------------- Road methods ---------------------

DSize Road::base_ = {0.4, 0.4};

Coord Road::GetLength() const {
    Coord res = end_.x - start_.x;
    if(IsVertical()){
        res = end_.y - start_.y;
    }
    return res;
}

DRectangle Road::CalcRectangle(){
    DPoint start { 
        start_.x > end_.x ? static_cast<double>(end_.x): static_cast<double>(start_.x),
        start_.y > end_.y ? static_cast<double>(end_.y) : static_cast<double>(start_.y)
    };
    DSize size {
        start_.x > end_.x ? static_cast<double>(start_.x) - end_.x : static_cast<double>(end_.x) - start_.x,
        start_.y > end_.y ? static_cast<double>(start_.y) - end_.y : static_cast<double>(end_.y) - start_.y
    };
    start.x -= base_.width;
    start.y -= base_.height;
    size.width += 2.0 * base_.width;
    size.height += 2.0 * base_.height;

    return {start, size};
}

bool Road::Contains(DPoint p) const{
    static double eps{1.e-9};
    return (p.x > (rect_.position.x - eps) && p.x < (rect_.position.x + rect_.size.width + eps)) &&
           (p.y > (rect_.position.y - eps) && p.y < (rect_.position.y + rect_.size.height + eps));
}

DCoord Road::GetLimit(DPoint p, DogDir dir) const{
    switch (dir)
    {
    case DogDir::NORTH: 
        return rect_.position.y;
    case DogDir::SOUTH:
        return rect_.position.y + rect_.size.height;
    case DogDir::EAST:
        return rect_.position.x + rect_.size.width;
    case DogDir::WEST:
        return rect_.position.x;
    }
    return 0.0;
}

// ============= Map methods -----------------------------------
    Map::Map(Id id, std::string name, double speed)
        : id_(std::move(id))
        , name_(std::move(name))
        , map_speed_(speed) {
    }

    const Map::Id& Map::GetId() const noexcept {
        return id_;
    }

    const std::string& Map::GetName() const noexcept {
        return name_;
    }

    double Map::GeSpeed() const noexcept {
        return map_speed_;
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

    void Map::AddLootType(const std::string& loot_types, int num) {
        loot_types_ = loot_types;
        loot_types_number_ = num;
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

DCoord Map::GetUpperLimit(DPoint p) const{
    DCoord limit{std::numeric_limits<double>::max()};
    for(const auto& road : roads_){
        if(road.Contains(p)){
            DCoord road_limit = road.GetLimit(p, DogDir::NORTH);
            if(road_limit < limit){
                limit = road_limit;
            }
        }
    }
    return limit;
}

DCoord Map::GetLeftLimit(DPoint p) const{
    DCoord limit{std::numeric_limits<double>::max()};
    for(const auto& road : roads_){
        if(road.Contains(p)){
            DCoord road_limit = road.GetLimit(p, DogDir::WEST);
            if(road_limit < limit){
                limit = road_limit;
            }
        }
    }
    return limit;
}

DCoord Map::GetBottomLimit(DPoint p) const{
    DCoord limit{std::numeric_limits<double>::lowest()};
    for(const auto& road : roads_){
        if(road.Contains(p)){
            DCoord road_limit = road.GetLimit(p, DogDir::SOUTH);
            if(road_limit > limit){
                limit = road_limit;
            }
        }
    }
    return limit;
}


DCoord Map::GetRightLimit(DPoint p) const{
    DCoord limit{std::numeric_limits<double>::lowest()};
    for(const auto& road : roads_){
        if(road.Contains(p)){
            DCoord road_limit = road.GetLimit(p, DogDir::EAST);
            if(road_limit > limit){
                limit = road_limit;
            }
        }
    }
    return limit;
}

DCoord Map::GetLimit(DPoint p, DogDir dir) const{
    switch (dir)
    {
    case DogDir::NORTH:
        return GetUpperLimit(p);
    case DogDir::SOUTH:
        return GetBottomLimit(p);
    case DogDir::WEST:
        return GetLeftLimit(p);
    case DogDir::EAST:
        return GetRightLimit(p);
    default:
        return std::numeric_limits<double>::max();
    }   
}

DPoint Map::GetRandomPoint(std::random_device& rd) const{
    DPoint road_point{0.0, 0.0};
//     // choose road
    size_t n_roads = GetRoads().size();
    std::uniform_int_distribution<std::mt19937_64::result_type> dist_roads(0, n_roads - 1);
    size_t road_index = dist_roads(rd);
    const Road& road = GetRoads()[road_index];
    // choose point on road
    Coord length = road.GetLength();
    //если length в метрах, то ставим точку с точностью до сантиметра
    int sign = length >=0 ? 1 : -1;
    std::uniform_int_distribution<std::mt19937_64::result_type> dist_length(0, std::abs(length) * 100);
    DCoord road_shift = sign * dist_length(rd) / 100.;
    std::cout << "road number: " << road_index << ", length: " << length << ", shift: " << road_shift << std::endl;
    road_point = road.IsVertical() ? DPoint{(double)road.GetStart().x, (double)road.GetStart().y + road_shift}
        : DPoint{(double)road.GetStart().x + road_shift, (double)road.GetStart().y};

    return road_point;
}
//-------------- GameSession -----------------------------------

GameSession::GameSession(model::Map* map, bool random_point, loot_gen::LootGenerator::TimeInterval loot_period, double loot_probability)
    : map_(map)
    , random_point_(random_point)
    , loot_generator_(loot_period, loot_probability, [this](){return this->generator();}) {
}

Dog* GameSession::AddDog(std::string_view dog_name){
    // DPoint p{0, 0};
    DPoint road_point{0.0, 0.0};
    if(random_point_) {
        road_point = map_->GetRandomPoint(random_device_);
        // // choose road
        // size_t n_roads = map_->GetRoads().size();
        // std::uniform_int_distribution<std::mt19937_64::result_type> dist_roads(0, n_roads - 1);
        // size_t road_index = dist_roads(random_device_);
        // const Road& road = map_->GetRoads()[road_index];
        // // choose point on road
        // Coord length = road.GetLength();
        // //если length в метрах, то ставим точку с точностью до сантиметра
        // std::uniform_int_distribution<std::mt19937_64::result_type> dist_length(0, length * 100);
        // DCoord road_shift = dist_length(random_device_) / 100.;
        // road_point = road.IsVertical() ? DPoint{(double)road.GetStart().x, (double)road.GetStart().y + road_shift}
        //     : DPoint{(double)road.GetStart().x + road_shift, (double)road.GetStart().y};
    }
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

double GameSession::generator(){
    std::uniform_int_distribution<std::mt19937_64::result_type> dist(0,1);

    return dist(random_device_);
}

void GameSession::GenerateLoots(std::chrono::milliseconds period){
    loot_gen::LootGenerator::TimeInterval n = period;
    unsigned n_loots = loots_.size();
    unsigned n_looters = dogs_.size();
    unsigned new_loots_number = loot_generator_.Generate(n, n_loots, n_looters);
    std::uniform_int_distribution<std::mt19937_64::result_type> dist(0,map_->GetLootTypesNumber() - 1);
    for(int i = 0; i < new_loots_number; ++i){
        LootData ld{dist(random_device_), map_->GetRandomPoint(random_device_)};
        std::cout << "new loot. Type: " << ld.first << ", {" << ld.second.x << "," << ld.second.y << "}\n";
        loots_.push_back(ld);
    }
}

//----------------- Game methods ------------------------------------

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
        join_session = &maps_sessions_[map_index]
            .emplace_back(&maps_[map_index], random_point_, GetLootPeriod(), GetLootProbability());
    }
    return {join_session->AddDog(std::string(dog_name)), join_session};
}

}  // namespace model

