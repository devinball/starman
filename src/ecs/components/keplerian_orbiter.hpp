#pragma once

#include "math/vector.hpp"
#include "ecs/ecs.hpp"

struct KeplerianOrbiter {
  double mass;
  double eccentricity;
  double semiMajorAxis;
  double inclination;
  double longAscendingNode;
  double argumentPeriapsis;
  double longPeriapsis;
  double orbitalPeriod;

  Vector3N barycenter;
  Entity parent;
};
