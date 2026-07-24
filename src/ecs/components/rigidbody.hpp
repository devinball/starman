#pragma once

#include "math/number.hpp"
#include "math/vector.hpp"
#include "math/quaternion.hpp"

struct Rigidbody {
    float mass;
    float momentInertia;

    Vector3N position;
    Vector3N velocity;
    Vector3N acceleration;

    QuaternionF rotation;

    // don't need a ton of precision, not more than c anyways

    Vector3N netTorque;
    Vector3N netForce;    
//    Number timeStep; // does not always have to be fixed maybe?
//    uint64_t lastPositionIndex; // index of the last position we reached
//    Trajectory trajectory; // list of 4-vectors that represents an objects path through spacetime
};
