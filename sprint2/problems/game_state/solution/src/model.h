#pragma once
#include <random>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unordered_map>
#include <deque>
#include <vector>

#include "tagged.h"
#include "Dog.h"
// #include "PlayerTokens.h"
// #include "GameSession.h"

namespace model {
using namespace std::literals;

class BadMapIdException : public std::invalid_argument {
public:
    BadMapIdException(const std::string& msg): std::invalid_argument(msg){}
};

using Dimension = int;
using Coord = Dimension;

struct Point {
    Coord x, y;
};

struct Size {
    Dimension width, height;
};

struct Rectangle {
    Rectangle(Point p, Size s): position(p), size(s){}
    Point position;
    Size size;
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
        , end_{end_x, start.y} {
    }

    Road(VerticalTag, Point start, Coord end_y) noexcept
        : start_{start}
        , end_{start.x, end_y} {
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

    Coord GetLength() const {
        Coord res = end_.x - start_.x;
        if(IsVertical()){
            res = end_.y - start_.y;
        }
        return res;
    }

private:
    // bool IsHorizontal(){
    //     return start_.y == end_.y;
    // }
    Point start_;
    Point end_;
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

class Map {
public:
    using Id = util::Tagged<std::string, Map>;
    using Roads = std::vector<Road>;
    using Buildings = std::vector<Building>;
    using Offices = std::vector<Office>;

    Map(Id id, std::string name) noexcept;

    const Id& GetId() const noexcept ;

    const std::string& GetName() const noexcept ;

    const Buildings& GetBuildings() const noexcept ;

    const Roads& GetRoads() const noexcept ;

    const Offices& GetOffices() const noexcept ;

    void AddRoad(const Road& road) ;

    void AddBuilding(const Building& building) ;

    void AddOffice(Office office);

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class GameSession {
public:
    GameSession(Map* map);
    Dog* AddDog(std::string_view dog_name);
    Map* GetMap();
    Dog* GetDogById(std::uint64_t id) ;
    size_t GetDogsNumber() const;
    const std::unordered_map<std::uint64_t, Dog*>& GetSessionDogs() const{
        return dogs_by_Ids_;
    }

private:
    std::random_device random_device_;
    std::deque<Dog> dogs_;
    std::unordered_map<std::uint64_t, Dog*> dogs_by_Ids_;
    Map* map_;
};

struct JoinResult{
    Dog* dog;
    GameSession* session;
};

class Game {
public:
    using Maps = std::vector<Map>;
    Game(int max_map_dogs = 10): max_map_dogs_(max_map_dogs){

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
    JoinResult JoinGame(std::string_view dog_name, std::string_view map_id_str);
private:
    int max_map_dogs_;
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
    std::unordered_map<size_t,std::deque<GameSession>> maps_sessions_;

};

}  // namespace model

