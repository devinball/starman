
/*
 I can have trajectory be an interface
 then implemebt it wilth newtonian, keplerian, etc
struct Trajectory {
  std::vector<State> states;

  void push(State state) {
    states.push_back(state);
  }

  override State getState(double t) = 0;
  
  State getStateAtTime(double t) {

  }

  State getClosestStateToTime(double t) {

  }

  int timeToIndex(double t) {

  }
};

struct NewtonianTrajectory : Trajectory {

}


// an orbit needs some sort of 'parent orbit' if it is to work
// in global coordinates

// a keplerian trajectory needs a series of orbits as
// well as knowing when to swap between them
// a trajectory also includes manuvers (future, current, and past)

// it needs to deal with all the celestial bodies
// soi is based on which body has the most gravitational
// influence at a specific point in spacetime.
// the boundary is where F_i = F_j
// it needs to handle all patched conics, not just one body

// TODO: create seperate trajectory and rigidbody components

// engines cannot fail when plottting a trajectory
// engines can only fail 'in the moment'

// direction may need to change along the length of the burn
// this could be accoumplished by having an 'angular momentum'
// so that the ship slowly rotates as it moves.

// the idea is a trajectory just acts like a function of time
// most of the state is internally managed.
// maybe we store time as an index? then i can just
// do time to index?

// if a trajectory is a polynomial, then
// i can do P(t) - Q(t), the take the square
// magnitude, which will be another polynomials
// which i can differentite and optimize.
*/
/*
struct Orbit {
  double eccentricity;
  double semiMajorAxis;
  double inclination;
  double longAscendingNode;
  double argumentPeriapsis;
  double longPeriapsis;
  double orbitalPeriod;

  // size and shape
  double eccentricity; // dimensionless
  double semiParameter; // meters
  double periapsis; // meters

  // rotation
  double inclination; // radians
  double longitudeAscendingNode; // radians
  double argumentPeriapsis; // radians

  // time describing
  double orbitalPeriod;

  Barycenter barycenter;

  // if end time is negative, or less than/equal to startTime then it
  // can be assumed that the orbit does not end
  double startTime;
  double endTime;

  enum OrbitType; // circular, eliptical, parabolic, hyperbolic
  OrbitType orbitType;
}
*/

#include "math/vector.hpp"
#include <vector>

struct Maneuver {
  float massFlowRate;
  float exhaustVelocity;
  float reliability;

  double startTime;
  double endTime;

  Vector3F direction;
};

struct Orbit {
  double start;

  virtual Vector3N position(float time) {};
  virtual Vector3N velocity(float time) {};
};

struct KeplerianOrbit : Orbit {
  // shape describing
  double eccentricity = 0;
  double semiLatusRectum = 0;

  // orientation describing
  double inclination = 0;
  double longAscendingNode = 0;
  double argumentPeriapsis = 0;

  // time describing
  double timeFromPeriapsis = 0;

  SphereOfInfluence soi;

  Vector3N position(float time) {
    // just return zero, orbiting around nothing makes no sense
    if (not this->soi) {
      return Vector3N(0, 0, 0);
    }

    t = time - (this->start - this->timeFromPeriapsis);

    M = meanAnomaly(orbit, t);
    theta = trueAnomaly(M, this->eccentricity);
    r = this->semiLatusRectum / (1 + this->eccentricity * cos(theta));
    o = times((cos(theta), sin(theta), 0), r);

    cos_ap, sin_ap = cos(this->argumentPeriapsis), sin(this->argumentPeriapsis);
    cos_lan, sin_lan = cos(this->longAscendingNode), sin(this->longAscendingNode);
    cos_inc, sin_inc = cos(this->inclination), sin(this->inclination);

    // i need to access the trajectory of the parent SOI
    return Vector3N(
      o[0]*(cos_ap*cos_lan - sin_ap*sin_lan*cos_inc) - o[1]*(sin_ap*cos_lan + cos_ap*sin_lan*cos_inc),
      o[0]*(cos_ap*sin_lan + sin_ap*cos_lan*cos_inc) + o[1]*(cos_ap*cos_lan*cos_inc - sin_ap*sin_lan),
      o[0]*sin_ap*sin_inc + o[1]*cos_ap*sin_inc
    ) + trajectory_pos(this->soi.trajectory, time);
  }

  Vector3N velocity(float time) {
    // just return zero, orbiting around nothing makes no sense
    if not this->soi:
      return (0, 0, 0)

    t = time - (this->start - this->time_from_peri)
    
    M = mean_anomaly(orbit, t)
    theta = true_anomaly(M, this->eccentricity)
    mu = this->soi.gravitational_parameter
    r = this->semi_latus_rectum / (1 + this->eccentricity * cos(theta))

    h = sqrt(mu * this->semi_latus_rectum)
    v_r = (mu / h) * this->eccentricity * sin(theta)
    v_theta = (mu / h) * (1 + this->eccentricity * cos(theta))
    o = (v_r * cos(theta) - v_theta * sin(theta), v_r * sin(theta) + v_theta * cos(theta), 0)

    cos_ap, sin_ap = cos(this->argument_periapsis), sin(this->argument_periapsis)
    cos_lan, sin_lan = cos(this->long_ascending_node), sin(this->long_ascending_node)
    cos_inc, sin_inc = cos(this->inclination), sin(this->inclination)

    return Vector3N(
      o[0]*(cos_ap*cos_lan - sin_ap*sin_lan*cos_inc) - o[1]*(sin_ap*cos_lan + cos_ap*sin_lan*cos_inc),
      o[0]*(cos_ap*sin_lan + sin_ap*cos_lan*cos_inc) + o[1]*(cos_ap*cos_lan*cos_inc - sin_ap*sin_lan),
      o[0]*sin_ap*sin_inc + o[1]*cos_ap*sin_inc
    ) + trajectory_vel(this->soi.trajectory, time);
  }
};

// when the trajectory is needed to extend, it does so switching
// between orbit types as needed. for now though, 100% keplerian
struct Trajectory {
  std::vector<Orbit> orbits;

  void extend() {

  }

  Vector3N position() {
    // get the orbit with the highest start time, that has already started at time
    orbits = [i for i in sorted(trajectory.orbits, key = lambda a : a.start) if i.start <= time]
    orbit = orbits[-1]
    return orbit_pos(orbit, time)  
  }

  Vector3N velocity() {
    orbits = [i for i in sorted(trajectory.orbits, key = lambda a : a.start) if i.start <= time]
    orbit = orbits[-1]
    return orbit_vel(orbit, time)
  }
};

struct Orbiter {
  Trajectory trajectory;
};

struct CelestialBody {
  double mass;
}

float aToP(a, e) {
  return a * (1 - e*e);
}

float hyperbolicEccentricAnomaly(M, e) {
  E = M; // initial guess for M
  
  float kepler(E) {
    return e * sinh(E) - E;
  }

  tolerance = 1e-10;
  maxIterations = 1000;
  iterations = 0;
  while (abs(kepler(E) - M) > tolerance && iterations < maxIterations) {
    E -= (kepler(E) - M) / (e * cosh(E) - 1);
    iterations += 1;
  }
  return E;
}

float elipticalEccentricAnomaly(M, e) {
  float E = M;
  
  float kepler(E) {
    return e * sin(E) - E;
  }

  tolerance = 1e-10;
  maxIterations = 1000;
  iterations = 0;
  while (abs(kepler(E) - M) > tolerance && iterations < maxIterations) {
    E -= (kepler(E) - M) / (e * cosh(E) - 1);
    iterations += 1;
  }
  return E;
}

float elipticalTrueAnomaly(M, e) {
  E = elipticalEccentricAnomaly(M, e);
  return 2 * atan(sqrt((1 + e) / (1 - e)) * tan(E / 2));
}

float parabolicTrueAnomaly(M, e) {
  D = cbrt(3 * M + sqrt(9 * M**2 + 2)) + cbrt(3 * M - sqrt(9 * M**2 + 1));
  return 2 * atan(D);
}

float hyperbolicTrueAnomaly(M, e) {
  E = hyperbolicEccentricAnomaly(M, e);
  return 2 * atan(sqrt((e + 1) / (e - 1)) * tanh(E / 2));
}

float meanAnomaly(float eccentricity, float semiLatusRectum, float time) {
  float M;
  float gravitationalParameter;
  if (eccentricity < 1) {
    a = semiLatusRectum / (1 - eccentricity**2);
    M = sqrt(gravitationalParameter / a**3) * time;
    M = fmod(M, 2 * pi);
  }
  else if (eccentricity == 1) {
    M = sqrt(gravitationalParameter / semiLatusRectum**3) * time;
  }
  else {
    a = semiLatusRectum / (eccentricity**2 - 1);
    M = sqrt(gravitationalParameter / abs(a)**3) * time;
  }

  return M;
}

float trueAnomaly(float M, float e) {
  float theta;
  if (e < 1) {
    theta = elipticalTrueAnomaly(M, e);
  }
  else if (e == 1) {
    theta = parabolicTrueAnomaly(M);
  }
  else {
    theta = hyperbolicTrueAnomaly(M, e);
  }

  return theta;
}

KeplerianOrbit orbitFromCartesian(Vector3N position, Vector3N velocity, float start, CelestialBody soi) {
  float gravitationalParameter;

  float h = pos.cross(vel);
  Vector3N eccV = vel.cross(h) / gravitationalParameter - pos.normalized();
  float ecc = magnitude(eccV);
  Vector3N n = Vector3N(-h[1], h[0], 0);

  float incl = acos(h[2] / magnitude(h));

  float longAscNode = 0;
  float argPeri = 0;
  float timeFromPeri = 0;

  if (magnitude(n) < 1e-8 * magnitude(h)) {
    longAscNode = 0
    if (ecc < 0.01) {
      argPeri = 0;
    }
    else {
      if (h[2] >= 0) {
        argPeri = atan2(eccV[1], eccV[0]);
      }
      else {
        argPeri = 2*pi - atan2(eccV[1], eccV[0]);
      }
    }
  }
  else {
    longAscNode = acos(n[0] / magnitude(n)) if n[1] >= 0 else (2 * pi - acos(n[0] / magnitude(n)));
    if (ecc < 0.01) {
      argPeri = 0;
    }
    else {
      argPeri = acos(dot(n, eccV) / (ecc * magnitude(n))) if eccV[2] >= 0 else (2 * pi - acos(dot(n, eccV) / (ecc * magnitude(n))));
    }
  }
 
  double semiMajorAxis = 1 / ((2 / magnitude(pos)) - (magnitude(vel)**2 / gravitationalParameter));
  double semiLatusRectum = magnitude(h)**2 / gravitationalParameter;

  float theta = acos(dot(eccV, pos) / (ecc * magnitude(pos))) if dot(pos, vel) >= 0 else (2 * pi - acos(dot(eccV, pos) / (ecc * magnitude(pos))));

  if (ecc < 1) {
    float E = 2 * atan(tan(theta / 2) / sqrt((1 + ecc) / (1 - ecc)))
    float M = E - ecc * sin(E)
    timeFromPeri = M / sqrt(gravitationalParameter / semiMajorAxis**3)
  }
  else if (ecc == 1) {
    float D = sqrt(2) * tan(theta / 2);
    float M = D + (D**3 / 3);
    timeFromPeri = M / sqrt(gravitationalParameter / semiLatusRectum**3);
  }
  else {
    float F = 2 * atanh(sqrt((ecc - 1) / (ecc + 1)) * tan(theta / 2));
    float M = ecc * sinh(F) - F;
    timeFromPeri = M / sqrt(gravitationalParameter / abs(semiMajorAxis)**3);
  }
  
  return KeplerianOrbit(
    ecc,
    semiLatusRectum,
    argument_periapsis=argPeri,
    inclination=incl,
    long_ascending_node=longAscNode,
    timeFromPeri=timeFromPeri,
    start=start,
    soi=soi
  );
}

float orbitDistance(Orbit a, Orbit b, float time) {
  return magnitude(a.position(time) - b.position(time));
}

CelestialBody getSoi(Vector3N position, float time) {

}

float soiHandoverTime(Orbit orbit) {

}

float orbitIntersectionTime(Orbit a, Orbit b, float margin) {

}
