#pragma once

#include "math/vector.hpp"
#include "math/constants.hpp"
#include "celestial_body.hpp"

#include <vector>
#include <optional>
#include <cfloat>
#include <functional>
#include <variant>

double aToP(double a, double e) {
  return a * (1 - e*e);
}

/*
double hyperbolicEccentricAnomaly(double M, double e) {
  if (!std::isfinite(M) || !(e > 1.0)) {
    printf("hyperbolicEccentricAnomaly: bad input M=%g e=%g\n", M, e);
    return M;
  }

  const double sign = M < 0 ? -1.0 : 1.0;
  const double m = std::abs(M);            // solve for m >= 0, restore the sign at the end
  if (m < 1e-15) return 0.0;

  auto f  = [&](double F) { return e * std::sinh(F) - F - m; };
  auto df = [&](double F) { return e * std::cosh(F) - 1.0; };

  // starting point: must satisfy f(F) >= 0, i.e. be at or right of the root
  // - sinh(F) - F >= F^3/6, so cbrt(6m) is an upper bound on the root
  // - for large m, the log form is much closer, and the loop below falls back to it
  double F = std::cbrt(6.0 * m);
  double Flog = std::log(2.0 * m / e + 1.8);
  if (f(Flog) >= 0.0 && Flog < F) F = Flog;
  while (f(F) < 0.0) F += 1.0;             // safety net, rarely runs

  for (int i = 0; i < 100; ++i) {
    double step = f(F) / df(F);
    F -= step;
    if (F < 0.0) F = 0.0;
    if (std::abs(step) <= 1e-14 * std::max(1.0, F)) break;   // relative tolerance
  }
  return sign * F;
}
*/

double hyperbolicEccentricAnomaly(double M, double e) {
  // this function has NAN problems
  double E = M; // initial guess for M
  
  auto kepler = [e](double E){ return e * std::sinh(E) - E; };

  double tolerance = 1e-10;
  int maxIterations = 1000;
  int iterations = 0;
  while (std::abs(kepler(E) - M) > tolerance && iterations < maxIterations) {
    E -= (kepler(E) - M) / (e * cosh(E) - 1);
    iterations += 1;
  }

  return E;
}

double elipticalEccentricAnomaly(double M, double e) {
  double E = M;

  auto kepler = [e](double E){ return E - e * std::sin(E); };

  // The nan issue starts here


  double tolerance = 1e-10;
  int maxIterations = 1000;
  int iterations = 0;
  while (std::abs(kepler(E) - M) > tolerance && iterations < maxIterations) {
    E -= (kepler(E) - M) / (1 - e * cos(E));
    iterations += 1;
  }
  return E;
}

double meanAnomaly(double eccentricity, double semiLatusRectum, double time, double gravitationalParameter) {
  double M;
  if (eccentricity < 1) {
    double a = semiLatusRectum / (1 - pow(eccentricity, 2));
    M = sqrt(gravitationalParameter / pow(a, 3)) * time;
    M = fmod(M, 2 * Constants::pi);
  }
  else if (eccentricity == 1) {
    M = sqrt(gravitationalParameter / pow(semiLatusRectum, 3)) * time;
  }
  else {
    double a = semiLatusRectum / (pow(eccentricity, 2) - 1);
    M = sqrt(gravitationalParameter / pow(std::abs(a), 3)) * time;
  }

  return M;
}

double trueAnomaly(double M, double e) {
  double theta;
  if (e < 1) {
    double E = elipticalEccentricAnomaly(M, e);
    theta = 2 * atan(sqrt((1 + e) / (1 - e)) * tan(E / 2));
    if (std::isnan(theta)) { printf("theta e\n"); }
  }
  else if (e == 1) {
    double D = cbrt(3 * M + sqrt(9 * pow(M, 2) + 2)) + cbrt(3 * M - sqrt(9 * pow(M, 2) + 1));
    theta = 2 * atan(D);
    if (std::isnan(theta)) { printf("theta p\n"); }
  }
  else {
    double E = hyperbolicEccentricAnomaly(M, e);
    theta = 2 * atan(sqrt((e + 1) / (e - 1)) * tanh(E / 2));
  }

  return theta;
}

struct Orbiter;

/*
struct ReferenceFrame {
  // will just point to an orbiter
  Trajectory& trajectory;
};
*/

// this will be changed eventually
using ReferenceFrame = Entity;

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

  double gravitationalParameter = 0;

  Vector3N position(double time) {
    double t = time - this->timeOfPeriapsis;

    double M = meanAnomaly(this->eccentricity, this->semiLatusRectum, t, gravitationalParameter);
    double theta = trueAnomaly(M, this->eccentricity);
    double r = this->semiLatusRectum / (1 + this->eccentricity * std::cos(theta));
    Vector3N o = Vector3N(std::cos(theta), std::sin(theta), 0) * r;

    double cos_ap = std::cos(this->argumentPeriapsis);
    double sin_ap = std::sin(this->argumentPeriapsis);
    double cos_lan = std::cos(this->longAscendingNode);
    double sin_lan = std::sin(this->longAscendingNode);
    double cos_inc = std::cos(this->inclination);
    double sin_inc = std::sin(this->inclination);
 
    return Vector3N(
      o.x*(cos_ap*cos_lan - sin_ap*sin_lan*cos_inc) - o.y*(sin_ap*cos_lan + cos_ap*sin_lan*cos_inc),
      o.x*(cos_ap*sin_lan + sin_ap*cos_lan*cos_inc) + o.y*(cos_ap*cos_lan*cos_inc - sin_ap*sin_lan),
      o.x*sin_ap*sin_inc + o.y*cos_ap*sin_inc
    );
  }

  Vector3N velocity(double time) {
    double t = time - this->timeOfPeriapsis;
    
    double M = meanAnomaly(this->eccentricity, this->semiLatusRectum, t, gravitationalParameter);
    double theta = trueAnomaly(M, this->eccentricity);
    double mu = gravitationalParameter;
    double r = this->semiLatusRectum / (1 + this->eccentricity * std::cos(theta));

    double h = sqrt(mu * this->semiLatusRectum);
    double v_r = (mu / h) * this->eccentricity * std::sin(theta);
    double v_theta = (mu / h) * (1 + this->eccentricity * std::cos(theta)); // TODO issue above here, if h is 0, the issue could occur
    Vector3N o = Vector3N(v_r * std::cos(theta) - v_theta * std::sin(theta), v_r * std::sin(theta) + v_theta * std::cos(theta), 0.0);

    double cos_ap = std::cos(this->argumentPeriapsis);
    double sin_ap = std::sin(this->argumentPeriapsis);
    double cos_lan = std::cos(this->longAscendingNode);
    double sin_lan = std::sin(this->longAscendingNode);
    double cos_inc = std::cos(this->inclination);
    double sin_inc = std::sin(this->inclination);

    return Vector3N(
      o.x*(cos_ap*cos_lan - sin_ap*sin_lan*cos_inc) - o.y*(sin_ap*cos_lan + cos_ap*sin_lan*cos_inc),
      o.x*(cos_ap*sin_lan + sin_ap*cos_lan*cos_inc) + o.y*(cos_ap*cos_lan*cos_inc - sin_ap*sin_lan),
      o.x*sin_ap*sin_inc + o.y*cos_ap*sin_inc
    );
  }
};

struct KinematicOrbit {
  // straight line with constant acceleration
  Vector3N pos;
  Vector3N vel;
  Vector3N acc;
  double initialTime;

  Vector3N position(double time) {
    return this->pos + this->vel * (time - this->initialTime) + this->acc * 0.5 * pow(time - this->initialTime, 2);
  }

  Vector3N velocity(double time) {
    return this->vel + this->acc * (time - this->initialTime);
  }
};

/*
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

struct RealTimeOrbit {
  // newtonian, controlled by player
  // in real time, no idea how to
  // implement this, or if it's actually
  // needed
};


*/

enum EndType {
  HANDOVER,
  HORIZON
};

using Orbit = std::variant<KeplerianOrbit, KinematicOrbit>;

struct Segment {
  double endTime;
  EndType endType;
  ReferenceFrame frame; // entt::null is world space
  Orbit orbit;
};

using Trajectory = std::vector<Segment>;

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
