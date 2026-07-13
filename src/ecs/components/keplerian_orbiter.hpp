#pragma once

#include "core/math/vector.hpp"

struct KeplerianOrbiter {
  double apoapsis;
  double periapsis;
  double eccentricity;
  double semiMajorAxis;
  double semiMinorAxis;
  double semiParameter;

  double inclination;
  double longAscendingNode;
  double argumentPeriapsis;
  double longPeriapsis;

  double meanMotion;
  double orbitalPeriod;

  Vector3 barycenter;
};
