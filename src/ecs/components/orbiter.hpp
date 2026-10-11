#pragma once

#include "math/vector.hpp"
#include "math/constants.hpp"
#include "celestial_body.hpp"

#include <vector>
#include <optional>
#include <cfloat>
#include <functional>
#include <variant>

struct Orbiter;

//using SphereOfInfluence = std::pair<std::shared_ptr<Orbiter>, std::shared_ptr<CelestialBody>>;

struct KeplerianOrbit {
  // shape describing
  double eccentricity = 0;
  double semiLatusRectum = 0;

  // orientation describing
  double inclination = 0;
  double longAscendingNode = 0;
  double argumentPeriapsis = 0;

  // time describing
  double timeOfPeriapsis = 0;

  Entity soi;
};

struct NewtonianOrbit {
  Vector3N position;
  Vector3N velocity;
};

struct ThrustingOrbit {
  Vector3N direction;
  float thrust;
};

struct NBodyOrbit {

};

using OrbitVariant = std::variant<KeplerianOrbit>;
using Segment = std::pair<double, OrbitVariant>;
using Trajectory = std::vector<Segment>;

struct InitialOrbiterState {
  Vector3N velocity;
};

struct Orbiter {
  Trajectory trajectory;
};

struct Maneuver {
  float massFlowRate;
  float exhaustVelocity;
  float reliability;

  double startTime;
  double endTime;

  Vector3F direction;
};

/*
Number orbitDistance(Orbit* a, Orbit* b, double time) {
  return (a->position(time) - b->position(time)).magnitude();
}

float orbitIntersectionTime(Orbit* a, Orbit* b, float margin) {

}
*/

/*
A trajectory represents a path thru global 3d space
  it's a piecewise function of orbits
an orbit is a small part of a trajectory
  an orbit requires something to orbit around.
  that thing is an orbiter, celestialbody pair
*/
