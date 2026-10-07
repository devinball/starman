#pragma once

#include "math/number.hpp"
#include "math/vector.hpp"
#include "math/quaternion.hpp"

#include <vector>

struct State {
  double time;
  
  float mass;
  float momentInertia;
  
  Vector3N position;
  Vector3N velocity;
  Vector3N acceleration;

  QuaternionF rotation;
};


struct Rigidbody {
  Trajectory trajectory;
  State currentState;
};

