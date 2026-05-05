#pragma once

#include "core/math/number.hpp"
#include "core/math/vector.hpp"
#include "core/math/quaternion.hpp"

struct Rigidbody {
    Number properTime;
    Number mass;
    Vector3 velocity;
    Quaternion angularVelocity; // maybe should be vector3
};
