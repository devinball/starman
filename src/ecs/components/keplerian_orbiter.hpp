#include "core/math/vector.hpp"

struct KeplerianOrbiter {
  float apoapsis;
  float periapsis;
  float eccentricity;
  float semiMajorAxis;
  float semiMinorAxis;
  float semiParameter;
  Vector3 barycenter;
};
