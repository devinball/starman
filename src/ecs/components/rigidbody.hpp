#pragma once

#include "core/math/number.hpp"
#include "core/math/vector.hpp"
#include "core/math/quaternion.hpp"

using Trajectory = std::vector<Vector4>;

struct Rigidbody {
    //Number properTime;
    float mass;

    Vector3 velocity; // Should not need high precision, highest value is c anyways
    Vector3 angularVelocity; // maybe should be vector3
    Matrix3x3F momentInertia;
//    Number timeStep; // does not always have to be fixed maybe?
//    uint64_t lastPositionIndex; // index of the last position we reached
//    Trajectory trajectory; // list of 4-vectors that represents an objects path through spacetime
};
