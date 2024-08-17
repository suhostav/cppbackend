#pragma once

#include <limits>

namespace model {

using Dimension = int;
using Coord = Dimension;
using DCoord = double;


struct Point {
    Coord x, y;
};

struct DPoint {
    DCoord x, y;
};

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
    DRectangle(DPoint p, DSize s): position(p), size(s) {}
    DPoint position;    //top left point of rectangle
    DSize size; //width, height from top left
};

enum class DogDir {
    NORTH = 'U',
    SOUTH = 'S',
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