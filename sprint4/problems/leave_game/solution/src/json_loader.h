#pragma once

#include <boost/json.hpp>
#include <filesystem>
#include <iostream>

#include "model.h"
#include "extra_data.h"

namespace json_loader {

model::Game LoadGame(const std::filesystem::path& json_path);
}  // namespace json_loader

std::ostream& operator<<(std::ostream& os, const model::Game& game);
