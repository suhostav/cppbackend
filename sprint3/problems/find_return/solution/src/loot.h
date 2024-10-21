#pragma once

#include "geom.h"

namespace model{

struct Loot {
    size_t id_;
    int type_;
    geom::Point2D pos_;
    bool taken = false;
    Loot(size_t id, int type, geom::Point2D pos)
        : id_(id), type_(type), pos_(pos) {}
    inline static size_t next_id_ = 0;
};

}