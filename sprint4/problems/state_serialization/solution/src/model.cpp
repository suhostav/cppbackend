#include "model.h"

#include <stdexcept>
#include <iostream>
#include <unordered_set>
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
    Point2D start { 
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

bool Road::Contains(Point2D p) const{
    static double eps{1.e-9};
    return (p.x > (rect_.position.x - eps) && p.x < (rect_.position.x + rect_.size.width + eps)) &&
           (p.y > (rect_.position.y - eps) && p.y < (rect_.position.y + rect_.size.height + eps));
}

DCoord Road::GetLimit(Point2D p, DogDir dir) const{
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
    Map::Map(Id id, std::string name, double speed, int bag_capacity)
        : id_(std::move(id))
        , name_(std::move(name))
        , map_speed_(speed)
        , bag_capacity_(bag_capacity) {
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

    void Map::AddLootType(const std::string& loot_types_str, std::vector<LootType>&& loot_types) {
        loot_types_ = loot_types_str;
        loot_types_objs_ = std::move(loot_types);
        loot_types_number_ = loot_types_objs_.size();
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

DCoord Map::GetUpperLimit(Point2D p) const{
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

DCoord Map::GetLeftLimit(Point2D p) const{
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

DCoord Map::GetBottomLimit(Point2D p) const{
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


DCoord Map::GetRightLimit(Point2D p) const{
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

DCoord Map::GetLimit(Point2D p, DogDir dir) const{
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

geom::Point2D Map::GetRandomPoint(std::random_device& rd) const{
    geom::Point2D road_point{0.0, 0.0};
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
    DCoord road_shift = static_cast<double>(dist_length(rd));
    road_shift = road_shift / 100. * sign;
    // std::cout << "road number: " << road_index << ", length: " << length << ", shift: " << road_shift << std::endl;
    road_point = road.IsVertical() ? geom::Point2D{(double)road.GetStart().x, (double)road.GetStart().y + road_shift}
        : geom::Point2D{(double)road.GetStart().x + road_shift, (double)road.GetStart().y};

    return road_point;
}
//-------------- GameSession -----------------------------------

GameSession::GameSession(model::Map* map, bool random_point, loot_gen::LootGenerator::TimeInterval loot_period, double loot_probability)
    : map_(map)
    , random_point_(random_point)
    , loot_generator_(loot_period, loot_probability, [this](){return this->generator();}) {
}

Dog* GameSession::AddDog(std::string_view dog_name){
    Point2D road_point{0.0, 0.0};
    if(random_point_) {
        road_point = map_->GetRandomPoint(random_device_);
    }
    Dog& new_dog = dogs_.emplace_back(dog_name, road_point, map_->GetBagCapacity());
    dogs_by_Ids_[new_dog.GetId()] = &new_dog;
    return &new_dog;
}

Dog* GameSession::AddDog(const Dog& dog){
    dogs_.push_back(dog);
    Dog* dog_ptr = &dogs_.back();
    dogs_by_Ids_[dog.GetId()] = &dogs_.back();
    return dog_ptr;
}

model::Map* GameSession::GetMap() const{
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
        loots_.emplace_back(Loot::next_id_++, dist(random_device_), map_->GetRandomPoint(random_device_));
    }
}

void GameSession::SetWidth(double dog_width, double loot_width, double office_width){
    dog_width_ = dog_width;
    loot_width_ = loot_width;
    office_width_ = office_width;
}

GameSession::EventsWithTypes GameSession::CreateSortedEventsList(){
    SessionItemGathererProvider take_provider(*this);
    std::vector<GatheringEvent> take_events = FindGatherEvents(take_provider);
    SessionOfficeGathererProvider drop_provider(*this);
    std::vector<GatheringEvent> drop_events = FindGatherEvents(drop_provider);
    EventsWithTypes all_events;
    all_events.reserve(take_events.size() + drop_events.size());
    for(size_t i = 0; i < take_events.size(); ++i){
        all_events.push_back({TAKE, take_events[i]});
    }
    for(size_t i = 0; i < drop_events.size(); ++i){
        all_events.push_back({DROP, drop_events[i]});
    }
    std::sort(all_events.begin(), all_events.end(), 
        [](const std::pair<EventType, GatheringEvent>& l, const std::pair<EventType, GatheringEvent>& r){
        return l.second.time < r.second.time;
    });
    return all_events;

}

void GameSession::CheckCollisions(){
    EventsWithTypes all_events = CreateSortedEventsList();

    for(const std::pair<EventType, GatheringEvent>& event : all_events){
        if(event.first == TAKE){
            TakeLoot(event.second);
        } else if(event.first == DROP){
            DropLoots(event.second);
        } else {
            throw std::logic_error("CheckCollision: invalid event type."s);
        }
    }
}

bool GameSession::TakeLoot(const GatheringEvent& take_event){

    auto& dog = dogs_[take_event.gatherer_id];
    bool result = dog.AddLoot(loots_[take_event.item_id]);
    if(result){
        loots_[take_event.item_id].taken = true;
    }
    
    return result; 
}

void GameSession::DropLoots(const GatheringEvent& drop_event){
    Loot& loot = loots_[drop_event.item_id];
    const LootType& loot_type = map_->GetLootTypesObjs()[loot.type_];
    int value = loot_type.value;
    Dog& dog = dogs_[drop_event.gatherer_id];
    dog.AddScore(value);
    dog.DropLoots();
}

void GameSession::ClearLoots(){
    if(loots_.size() > 0) {
        loots_.erase(std::remove_if(loots_.begin(), loots_.end(), [](const Loot& loot){
            return loot.taken;
        }), loots_.end());
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

Map* Game::GetMap(const std::string& map_id_str){
    Map::Id map_id{std::string(map_id_str)};
    size_t map_ind = GetMapIndex(map_id);
    return &maps_[map_ind];
}

JoinResult Game::JoinGame(std::string_view dog_name, std::string_view map_id_str){
    Map::Id map_id{std::string(map_id_str)};
    size_t map_index = GetMapIndex(map_id);
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
        join_session->SetWidth(0.6, 0, 0.5);
    }
    return {join_session->AddDog(std::string(dog_name)), join_session};
}

GameSession* Game::CreateSession(std::string map_id_str){
    Map::Id map_id{std::string(map_id_str)};
    size_t map_index = GetMapIndex(map_id);
    std::deque<GameSession>& map_sessions = maps_sessions_[map_index];
    GameSession* session = &map_sessions.emplace_back(&maps_[map_index], random_point_, GetLootPeriod(), GetLootProbability());
    return session;
}

}  // namespace model

