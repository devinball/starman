#pragma once

#include "core/math/number.hpp"
#include "core/math/vector.hpp"
#include "core/math/quaternion.hpp"

struct Rigidbody {
    Number mass;
    Vector3 velocity;
    Quaternion angularVelocity;
};
