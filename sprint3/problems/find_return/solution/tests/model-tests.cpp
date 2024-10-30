#include <cmath>
#include <catch2/catch_test_macros.hpp>

#include "../src/loot_generator.h"
#include "../src/model.h"

using namespace std::literals;

SCENARIO("Genegate loots"){
    using loot_gen::LootGenerator;
    using TimeInterval = LootGenerator::TimeInterval;
    GIVEN("a gamesession"){
        // LootGenerator gen{1s, 1.0};
        model::Map::Id map_id{"map1"s};
        model::Map map(map_id, "Map1", 1, 3);
        map.AddRoad(model::Road(model::Road::HORIZONTAL, {0,0}, 1));
        model::GameSession session(&map, false, 1s, 1.0);
        session.AddDog("Sharik"s);
        session.AddDog("Tusik"s);
        WHEN("time is greater than base interval") {
            session.GenerateLoots(2s);
            THEN("number of generated loot is increased"){
                CHECK(session.GetLoots().size() == static_cast<size_t>(2));
            }
        }
        
    }
}