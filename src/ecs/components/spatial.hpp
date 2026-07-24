#pragma once

#include "math/vector.hpp"
#include "math/quaternion.hpp"

// a simple component to represent an object in 3d space
struct Spatial {
    Vector3N position;
    Vector3F scale;
    QuaternionF rotation;
};
