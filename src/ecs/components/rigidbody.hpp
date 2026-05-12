#pragma once

#include "core/math/number.hpp"
#include "core/math/vector.hpp"
#include "core/math/quaternion.hpp"

struct Rigidbody {
    Number properTime;
    Number mass;
    Vector3 velocity; // Should not need high precision, highest value is c anyways
    QuaternionF angularVelocity; // maybe should be vector3
};
