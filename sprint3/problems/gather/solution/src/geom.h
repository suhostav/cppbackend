#pragma once

namespace geom{

struct Point2D {
    double x, y;
};

inline bool operator==(const Point2D& p1, const Point2D& p2){
    return p1.x == p2.x && p1.y == p2.y;
}

}   //geom