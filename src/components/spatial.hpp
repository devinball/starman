#pragma once

#include "core/math/vector.hpp"
#include "core/math/quaternion.hpp"

// a simple component to represent an object in 3d space
struct Spatial {
    BigVector3 position;
    Quaternion rotation;
    Vector3 scale;
};