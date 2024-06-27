#pragma once

#include <boost/json.hpp>
#include <filesystem>
#include <iostream>

#include "model.h"

namespace json_loader {

using namespace boost::json;
using namespace std::literals;

model::Game LoadGame(const std::filesystem::path& json_path);
}  // namespace json_loader

std::ostream& operator<<(std::ostream& os, const model::Game& game);
