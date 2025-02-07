#pragma once

#include <boost/json.hpp>
#include <filesystem>
#include <iostream>

#include "model.h"
#include "extra_data.h"

namespace json_loader {
void LoadRoads(model::Map& map, const boost::json::array& jroads);
void LoadBuildings(model::Map& map, const boost::json::array& jbuildings);
void LoadOffices(model::Map& map, const boost::json::array& joffices);
void LoadLootTypes(model::Map& map, const boost::json::array& jloot_types);
void LoadMaps(model::Game& game, const boost::json::array& jmaps, double default_speed, int default_bag_capacity);
model::Game LoadGame(const std::filesystem::path& json_path);
}  // namespace json_loader

std::ostream& operator<<(std::ostream& os, const model::Game& game);
