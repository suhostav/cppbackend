#pragma once
#include <chrono>
#include <random>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unordered_map>
#include <deque>
#include <vector>

#include "tagged.h"
#include "coords.h"
#include "geom.h"
#include "Dog.h"
#include "loot.h"
#include "loot_generator.h"
#include "collision_detector.h"

namespace model {
using namespace std::literals;
using namespace std::chrono_literals;

class BadMapIdException : public std::invalid_argument {
public:
    BadMapIdException(const std::string& msg): std::invalid_argument(msg){}
};


struct Offset {
    Dimension dx, dy;
};

class Road {
    struct HorizontalTag {
        explicit HorizontalTag() = default;
    };

    struct VerticalTag {
        explicit VerticalTag() = default;
    };

public:
    constexpr static HorizontalTag HORIZONTAL{};
    constexpr static VerticalTag VERTICAL{};

    Road(HorizontalTag, Point start, Coord end_x) noexcept
        : start_{start}
        , end_{end_x, start.y}
        , rect_{CalcRectangle()} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y}
        , rect_{CalcRectangle()}  {
    }

    bool IsHorizontal() const noexcept {
        return start_.y == end_.y;
    }

    bool IsVertical() const noexcept {
        return start_.x == end_.x;
    }

    Point GetStart() const noexcept {
        return start_;
    }

    Point GetEnd() const noexcept {
        return end_;
    }

    Coord GetLength() const;
    const DRectangle& GetRectangle() const {
        return rect_;
    }
    bool Contains(Point2D p) const;
    DCoord GetLimit(Point2D p, DogDir dir) const;

private:
    DRectangle CalcRectangle();

    Point start_;
    Point end_;
    DRectangle rect_;
    static DSize base_;
};

class Building {
public:
    explicit Building(Rectangle bounds) noexcept
        : bounds_{bounds} {
    }

    const Rectangle& GetBounds() const noexcept {
        return bounds_;
    }

private:
    Rectangle bounds_;
};

class Office {
public:
    using Id = util::Tagged<std::string, Office>;

    Office(Id id, Point position, Offset offset) noexcept
        : id_{std::move(id)}
        , position_{position}
        , offset_{offset} {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    Point GetPosition() const noexcept {
        return position_;
    }

    Offset GetOffset() const noexcept {
        return offset_;
    }

private:
    Id id_;
    Point position_;
    Offset offset_;
};

// struct LootType{
//     std::string name;
//     std::string file;
//     std::string type;
//     int rotation;
//     std::string color;
//     double scale;
// };

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;
    // using LootTypes = std::vector<LootType>;

    Map(Id id, std::string name, double speed, int bag_capacity);

    const Id& GetId() const noexcept;

    const std::string& GetName() const noexcept;

    double GeSpeed() const noexcept;

    const Buildings& GetBuildings() const noexcept;

    const Roads& GetRoads() const noexcept;

    const Offices& GetOffices() const noexcept;

    void AddRoad(const Road& road);

    void AddBuilding(const Building& building);

    void AddOffice(Office office);

    // void AddLootType(const LootType& loot_type);
    void AddLootType(const std::string& loot_types_str, std::vector<LootType>&& loot_types);

    DCoord GetLimit(Point2D p, DogDir dir) const;

    geom::Point2D GetRandomPoint(std::random_device& rd) const;
    const std::string& GetLootTypes() const {
        return loot_types_;
    }
    int GetLootTypesNumber() const {
        return loot_types_number_;
    }

    int GetBagCapacity() const {
        return bag_capacity_;
    }
    
    int GetLootTypeValue(size_t ind) const {
        return loot_types_objs_[ind].value;
    }

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    DCoord GetUpperLimit(Point2D p) const;
    DCoord GetBottomLimit(Point2D p) const;
    DCoord GetLeftLimit(Point2D p) const;
    DCoord GetRightLimit(Point2D p) const;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;
    double map_speed_;
    int bag_capacity_;
    std::string loot_types_;
    std::vector<LootType> loot_types_objs_;
    int loot_types_number_ = 0;


    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

using namespace collision_detector;

class GameSession {
public:
    using Loots = std::deque<Loot>;
    using Gatherers = std::vector<Gatherer>;
    using Items = std::vector<Item>;
    enum EventType {
        TAKE,
        DROP
    };
    using EventsWithTypes = std::vector<std::pair<EventType, GatheringEvent>>;

    class SessionItemGathererProvider: public ItemGathererProvider{
    public:
        SessionItemGathererProvider(GameSession& session): session_(session) {}
        size_t ItemsCount() const override{
            return session_.GetLoots().size();
        }
        Item GetItem(size_t idx) const override {

            const auto& loot = session_.GetLoots().at(idx);
            return {{loot.pos_.x, loot.pos_.y}, session_.GetLootWidth()};
        }
        size_t GatherersCount() const override {
            return session_.GetDogs().size();
        }
        Gatherer GetGatherer(size_t idx) const override {
            const auto& dog = session_.GetDogs().at(idx);
            return {dog.GetPrevPoint2D(), dog.GetPoint2D(), session_.GetDogWidth() / 2.};
        }
    private:
        GameSession& session_;
    };

    class SessionOfficeGathererProvider: public ItemGathererProvider{
    public:
        SessionOfficeGathererProvider(GameSession& session): session_(session) {}
        size_t ItemsCount() const override{
            return session_.GetMap()->GetOffices().size();
        }
        Item GetItem(size_t idx) const override {
            const Office& office = session_.GetMap()->GetOffices().at(idx);
            return {{static_cast<double>(office.GetPosition().x), static_cast<double>(office.GetPosition().y)}, 
                session_.GetOfficeWidth() / 2.0};
        }
        size_t GatherersCount() const override {
            return session_.GetDogs().size();
        }
        Gatherer GetGatherer(size_t idx) const override {
            const auto& dog = session_.GetDogs().at(idx);
            return {dog.GetPrevPoint2D(), dog.GetPoint2D(), session_.GetDogWidth() / 2.};
        }
    private:
        GameSession& session_;
    };

    GameSession(Map* map, bool random_point, 
        loot_gen::LootGenerator::TimeInterval loot_period, 
        double loot_probability,
        std::chrono::seconds dog_retirement_time);
    Dog* AddDog(std::string_view dog_name);
    Dog* AddDog(const Dog& dog);

    std::deque<Dog> GetDogs() const {
        return dogs_;
    }
    Map* GetMap() const;
    void SetMap(Map* map){
        map_ = map;
    }
    Dog* GetDogById(std::uint64_t id) ;
    bool IsRandomPoint() const {
        return random_point_;
    }
    size_t GetDogsNumber() const;
    const std::unordered_map<std::uint64_t, Dog*>& GetSessionDogs() const{
        return dogs_by_Ids_;
    }
    void GenerateLoots(std::chrono::milliseconds period);
    void RestoreLoot(size_t id, int type, geom::Point2D pos);
    const Loots GetLoots() const {
        return loots_;
    }
    void RemoveDog(Dog& dog);
    //------------обработка коллизий ----------------
    void SetWidth(double dog_width, double loot_width, double office_width); 
    double GetDogWidth() const { return dog_width_; }
    double GetLootWidth() const { return loot_width_; }
    double GetOfficeWidth() const { return office_width_; }
    void CheckCollisions();
    void ClearLoots();
private:
    double generator();
    EventsWithTypes CreateSortedEventsList();
    bool TakeLoot(const GatheringEvent& take_event);
    void DropLoots(const GatheringEvent& drop_event);
    std::random_device random_device_;
    std::deque<Dog> dogs_;
    double dog_width_ = default_dog_width;
    double loot_width_ = default_loot_width;
    double office_width_ = default_office_width;
    std::unordered_map<std::uint64_t, Dog*> dogs_by_Ids_;
    Loots loots_;
    Map* map_;
    bool random_point_;
    loot_gen::LootGenerator loot_generator_;
    inline static double default_dog_width = 0.6;
    inline static double default_loot_width = 0.0;
    inline static double default_office_width = 0.5;
    std::chrono::seconds dog_retirement_time_;
};

struct JoinResult{
    Dog* dog;
    GameSession* session;
};

class Game {
public:
    using Maps = std::vector<Map>;
    Game(int max_map_dogs = 10)
        : max_map_dogs_(max_map_dogs){

    }

    void AddMap(Map map);

    const Maps& GetMaps() const noexcept {
        return maps_;
    }

    const Map* FindMap(const Map::Id& id) const noexcept {
        if (auto it = map_id_to_index_.find(id); it != map_id_to_index_.end()) {
            return &maps_.at(it->second);
        }
        return nullptr;
    }

    size_t GetMapIndex(const Map::Id& id){
        if(map_id_to_index_.count(id) > 0){
            return map_id_to_index_.at(id);
        }
        throw BadMapIdException("Bad map id"s + *id);
    }
    void SetRandomPoint(bool random_point){
        random_point_ = random_point;
    }
    bool GetRandomPoint() const {
        return random_point_;
    }
    Map* GetMap(const std::string& map_id_str);
    JoinResult JoinGame(std::string_view dog_name, std::string_view map_id_str);
    GameSession* CreateSession(std::string map_id_str);
    void SetLootPeriod(double p){
        loot_period_ = std::chrono::duration_cast<std::chrono::milliseconds>(1ms * static_cast<int>(p * 1000));
    }
    void SetLootProbability(double p){
        loot_probability_ = p;
    }
    loot_gen::LootGenerator::TimeInterval GetLootPeriod() const {
        return loot_period_;
    }
    double GetLootProbability(){
        return loot_probability_;
    }
    void SetDogRetirementTime(int64_t period){
        dog_retirement_time = period;
    }
    std::chrono::seconds GetDogRetirementTime() const {
        return duration_cast<std::chrono::seconds>(1s * dog_retirement_time);
    }
private:
    int max_map_dogs_;
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    std::unordered_map<size_t,std::deque<GameSession>> maps_sessions_;
    bool random_point_ = false;
    loot_gen::LootGenerator::TimeInterval loot_period_ = 1000s;
    double loot_probability_ = 0;
    int64_t dog_retirement_time = 10;
};

}  // namespace model

