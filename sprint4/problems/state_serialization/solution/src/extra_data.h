#pragma once

#include <map>
#include <string>
#include <vector>
#include <boost/json.hpp>
#include "model.h"

class ExtraData {
public:
    void Add(const model::Map::Id& id, boost::json::array&& lt){
        loot_types_[id] = std::move(lt);
    }
    const boost::json::array& GetLootTypes(model::Map::Id id) const {
        using namespace std::literals;
        if(loot_types_.contains(id)){
            return loot_types_.at(id);
        }
        throw std::invalid_argument("Invalid map id :"s + *id);
    }
private:
    std::map<model::Map::Id, boost::json::array>loot_types_;
};