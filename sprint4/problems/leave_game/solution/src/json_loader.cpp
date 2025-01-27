#include "json_loader.h"
#include <fstream>
#include <sstream>

namespace json_loader {

using namespace boost::json;
using namespace std::literals;

int to_int(int64_t i){
    return static_cast<int>(i);
}

model::Game LoadGame(const std::filesystem::path& json_path) {
    // Загрузить содержимое файла json_path, например, в виде строки
    // Распарсить строку как JSON, используя boost::json::parse
    // Загрузить модель игры из файла
    model::Game game;
    std::ifstream in{json_path};
    if(!in){
        std::cerr << "Open file error. File: " << json_path << std::endl;
        return {};
    }
    std::stringstream ss;
    ss << in.rdbuf();
    auto data = parse(ss.str());
    auto data_object = data.as_object();
    double default_speed = 0;
    if(data_object.contains("defaultDogSpeed"s)){
        default_speed = data_object.at("defaultDogSpeed"s).as_double();
    }
    int default_bag_capacity = 3;
    if(data_object.contains("defaultBagCapacity"s)){
        default_bag_capacity = data_object.at("defaultBagCapacity"s).as_int64();
    }
    if(data_object.contains("lootGeneratorConfig"s)){
        auto jloots = data_object.at("lootGeneratorConfig");
        game.SetLootPeriod(jloots.at("period").as_double());
        game.SetLootProbability(jloots.at("probability").as_double());
    }
    if(!data_object.contains("maps"s)){
        std::cout << "Error reading config\n";
        return game;
    }
    auto jmaps = data_object.at("maps").as_array();
    for(auto jmap : jmaps){
        model::Map::Id id{std::string{jmap.at("id").as_string()}};
        std::string name{jmap.at("name"s).as_string()};
        auto jroads = jmap.at("roads").as_array();
        double map_speed = default_speed;
        if(jmap.as_object().contains("dogSpeed"s)){
            map_speed = jmap.at("dogSpeed"s).as_double();
        }
        int bag_capacity = default_bag_capacity;
        if(jmap.as_object().contains("bagCapacity"s)){
            map_speed = jmap.at("bagCapacity"s).as_int64();
        }

        model::Map map{id, name, map_speed, bag_capacity};

        for(const auto& jroad : jroads){
            if(jroad.as_object().contains("x1")){
                map.AddRoad(model::Road{model::Road::HORIZONTAL, 
                    {to_int(jroad.at("x0").as_int64()), to_int(jroad.at("y0").as_int64())}, 
                    to_int(jroad.at("x1").as_int64())});
            } else {
                map.AddRoad(model::Road{model::Road::VERTICAL, 
                    {to_int(jroad.at("x0").as_int64()), to_int(jroad.at("y0").as_int64())}, 
                    to_int(jroad.at("y1").as_int64())});
            }
        }
        auto jbuildings = jmap.at("buildings").as_array();
        for(const auto& jbuilding : jbuildings){
            map.AddBuilding(model::Building{model::Rectangle{{to_int(jbuilding.at("x").as_int64()), to_int(jbuilding.at("y").as_int64())},
                {to_int(jbuilding.at("w").as_int64()), to_int(jbuilding.at("h").as_int64())}}});
        }
        auto joffices = jmap.at("offices").as_array();
        for(const auto& joffice : joffices){
            model::Office::Id id{std::string{joffice.at("id").as_string()}};
            map.AddOffice(model::Office{
                id,
                {to_int(joffice.at("x").as_int64()), to_int(joffice.at("y").as_int64())},
                {to_int(joffice.at("offsetX").as_int64()), to_int(joffice.at("offsetY").as_int64())}
            });
        }
        if(jmap.as_object().contains("lootTypes")) {
            boost::json::array jloot_types = jmap.at("lootTypes").as_array();
            std::vector<model::LootType> loot_types;
            for(const auto& jloot_type : jloot_types){
                if(jloot_type.as_object().contains("value")) {
                    loot_types.push_back(model::LootType{.value = static_cast<int>(jloot_type.at("value").as_int64())});
                }
            }
            map.AddLootType(boost::json::serialize(jloot_types), std::move(loot_types));
        }
        game.AddMap(map);
    }
    return game;
}


}  // namespace json_loader

std::ostream& operator<<(std::ostream& os, const model::Game& game){
    os << "Maps: \n";
    auto maps = game.GetMaps();
    for(const auto& map : maps){
        os << *map.GetId() << ": " << "name \n"; 
        os << "\tRoads: \n";
        auto roads = map.GetRoads();
        for(auto road : roads){
            os << "\t\t{" << road.GetStart().x << ", " << road.GetStart().y << "}, {"
                << road.GetEnd().x << ", " << road.GetEnd().y << "}\n";
        }
        auto buildings = map.GetBuildings();
        os << "\tBuildings: \n";
        for(auto building : buildings){
            os << "\t\t{" << building.GetBounds().position.x
               << ", " << building.GetBounds().position.y << "}, {"
               << building.GetBounds().size.width << ", " << building.GetBounds().size.height << "}\n";
        }
        auto offices = map.GetOffices();
        os << "\tOffices: \n";
        for(auto office : offices){
            os << "\t\t" << *office.GetId() << ": {"
               << office.GetPosition().x << ", " << office.GetPosition().y << "}, {"
               << office.GetOffset().dx << ", " <<office.GetOffset().dy << "}\n";
        }
    }
    return os;
}