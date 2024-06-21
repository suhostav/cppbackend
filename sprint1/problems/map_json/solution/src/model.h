#pragma once
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "tagged.h"

namespace model {

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

    std::string ToJson() const{
        std::stringstream ss;
        ss << "{ ";
        if(IsHorizontal()){
            ss << "\"x0\": "<< start_.x
               << ", \"y0\": " << start_.y
               << ", \"x1\": " << end_.x;
        } else {
            ss << "\"x0\": "<< start_.x
               << ", \"y0\": " << start_.y
               << ", \"y1\": " << end_.y;
        }
        ss << " }";
        return ss.str();
    }

private:
    bool IsHorizontal(){
        return start_.y == end_.y;
    }
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

    std::string ToJson() const{
        std::stringstream ss;
        ss << "{ ";
        ss << "\"x\": " << bounds_.position.x
           << ", \"y\": " << bounds_.position.y
           << ", \"w\": " << bounds_.size.width
           << ", \"h\": " << bounds_.size.height
           << " }";
        return ss.str();      
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

    std::string ToJson(){
        std::stringstream ss;
        ss << "{ ";
        ss << "\"id\": \"" << *id_ 
           << "\", \"x\": " << position_.x
           << ", \"y\": " << position_.y
           << ", \"offsetX\": " << offset_.dx
           << ", \"offsetY\": " << offset_.dy
           << " }";
        return ss.str();
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

    Map(Id id, std::string name) noexcept
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const Id& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

    const Buildings& GetBuildings() const noexcept {
        return buildings_;
    }

    const Roads& GetRoads() const noexcept {
        return roads_;
    }

    const Offices& GetOffices() const noexcept {
        return offices_;
    }

    void AddRoad(const Road& road) {
        roads_.emplace_back(road);
    }

    void AddBuilding(const Building& building) {
        buildings_.emplace_back(building);
    }

    void AddOffice(Office office);

    std::string ToJson() const{
        using namespace std::literals;
        std::stringstream ss;
        ss << "{\n";
        ss << "  \"id\": \"" << *id_ << "\",\n";
        ss << "  \"name\": \"" << name_ << "\",\n";
        ss << "  \"roads\": ["s;
        auto roads = GetRoads();
        bool first = true;
        for(auto road : roads){
            if(first){ first = false; } else { ss << ','; }
            ss << "\n    ";
            ss << road.ToJson();
        }
        ss << "\n  ],\n";
        auto buildings = GetBuildings();
        ss << "  \"buildings\": ["s;
        first = true;
        for(auto building : buildings){
            if(first){ first = false; } else { ss << ','; }
            ss << "\n    ";
            ss << building.ToJson();
        }
        ss << "\n  ],";
        auto offices = GetOffices();
        ss << "\n  \"offices\": ["s;
        first = true;
        for(auto office : offices){
            if(first){ first = false; } else { ss << ','; }
            ss << "\n    ";
            ss << office.ToJson();
        }
        ss << "\n  ]";
        ss << "\n}";
        return ss.str();
    }

private:
    using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

    Id id_;
    std::string name_;
    Roads roads_;
    Buildings buildings_;

    OfficeIdToIndex warehouse_id_to_index_;
    Offices offices_;
};

class Game {
public:
    using Maps = std::vector<Map>;

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
private:
    using MapIdHasher = util::TaggedHasher<Map::Id>;
    using MapIdToIndex = std::unordered_map<Map::Id, size_t, MapIdHasher>;

    std::vector<Map> maps_;
    MapIdToIndex map_id_to_index_;
};

}  // namespace model
