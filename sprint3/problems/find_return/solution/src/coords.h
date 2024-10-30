#pragma once

#include <limits>
#include "geom.h"

namespace model {

using Dimension = int;
using Coord = Dimension;
using DCoord = double;

using namespace geom;

struct Point {
    Coord x, y;
};

// struct DPoint {
//     DCoord x, y;
//     operator geom::Point2D(){
//         return {x,y};
//     }
// };


struct Size {
    Dimension width, height;
};

struct DSize {
    double width, height;
};

struct Rectangle {
    Rectangle(Point p, Size s): position(p), size(s){}
    Point position;
    Size size;
};

struct DRectangle{
    DRectangle(Point2D p, DSize s): position(p), size(s) {}
    Point2D position;    //top left point of rectangle
    DSize size; //width, height from top left
};

enum class DogDir {
    NORTH = 'U',
    SOUTH = 'D',
    WEST = 'L',
    EAST = 'R'
};

using DogSpeedPerSecond = double;

struct DogSpeed {
    DogSpeed(DogSpeedPerSecond h, DogSpeedPerSecond v): hs(h), vs(v){}
    DogSpeedPerSecond hs;    //horizontal speed
    DogSpeedPerSecond vs;    //vertical speed
};


}   //namespace model