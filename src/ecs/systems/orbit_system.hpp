#pragma once

#include "ecs/system.hpp"
#include "ecs/components/orbiter.hpp"
#include "ecs/components/spatial.hpp"

#include "imgui.h"

struct OrbitSystem : System {
  Trajectory soiTrajectory(Entity soi) {
    return context->registry->get<Orbiter>(soi).trajectory;
  }

  double soiGravitationalParameter(Entity soi) {
    return context->registry->get<CelestialBody>(soi).gravitationalParameter;
  }

  double getSoiRadius(Entity soi) {
    return context->registry->get<CelestialBody>(soi).soiRadius;
  }

  float aToP(float a, float e) {
    return a * (1 - e*e);
  }

  float hyperbolicEccentricAnomaly(float M, float e) {
    float E = M; // initial guess for M
    
    auto kepler = [e](float E){ return e * std::sinh(E) - E; };

    float tolerance = 1e-10;
    int maxIterations = 1000;
    int iterations = 0;
    while (abs(kepler(E) - M) > tolerance && iterations < maxIterations) {
      E -= (kepler(E) - M) / (e * cosh(E) - 1);
      iterations += 1;
    }
    return E;
  }

  float elipticalEccentricAnomaly(float M, float e) {
    float E = M;

    auto kepler = [e](float E){ return E - e * std::sin(E); };

    float tolerance = 1e-10;
    int maxIterations = 1000;
    int iterations = 0;
    while (abs(kepler(E) - M) > tolerance && iterations < maxIterations) {
      E -= (kepler(E) - M) / (1 - e * cos(E));
      iterations += 1;
    }
    return E;
  }

  float elipticalTrueAnomaly(float M, float e) {
    float E = elipticalEccentricAnomaly(M, e);
    return 2 * atan(sqrt((1 + e) / (1 - e)) * tan(E / 2));
  }

  float parabolicTrueAnomaly(float M) {
    float D = cbrt(3 * M + sqrt(9 * pow(M, 2) + 2)) + cbrt(3 * M - sqrt(9 * pow(M, 2) + 1));
    return 2 * atan(D);
  }

  float hyperbolicTrueAnomaly(float M, float e) {
    float E = hyperbolicEccentricAnomaly(M, e);
    return 2 * atan(sqrt((e + 1) / (e - 1)) * tanh(E / 2));
  }

  float meanAnomaly(double eccentricity, double semiLatusRectum, double time, double gravitationalParameter) {
    float M;
    if (eccentricity < 1) {
      float a = semiLatusRectum / (1 - pow(eccentricity, 2));
      M = sqrt(gravitationalParameter / pow(a, 3)) * time;
      M = fmod(M, 2 * Constants::pi);
    }
    else if (eccentricity == 1) {
      M = sqrt(gravitationalParameter / pow(semiLatusRectum, 3)) * time;
    }
    else {
      float a = semiLatusRectum / (pow(eccentricity, 2) - 1);
      M = sqrt(gravitationalParameter / pow(abs(a), 3)) * time;
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

  Vector3N orbitPosition(const KeplerianOrbit& orbit, double orbitStart, double time) {
    // just return zero, orbiting around nothing makes no sense
    if (orbit.soi == entt::null) {
      return Vector3N(0, 0, 0);
    }

    double gravitationalParameter = soiGravitationalParameter(orbit.soi);

    double t = time - (orbitStart - orbit.timeFromPeriapsis);

    float M = meanAnomaly(orbit.eccentricity, orbit.semiLatusRectum, t, gravitationalParameter);
    float theta = trueAnomaly(M, orbit.eccentricity);
    float r = orbit.semiLatusRectum / (1 + orbit.eccentricity * std::cos(theta));
    Vector3N o = Vector3N(std::cos(theta), std::sin(theta), 0) * r;

    float cos_ap = std::cos(orbit.argumentPeriapsis);
    float sin_ap = std::sin(orbit.argumentPeriapsis);
    float cos_lan = std::cos(orbit.longAscendingNode);
    float sin_lan = std::sin(orbit.longAscendingNode);
    float cos_inc = std::cos(orbit.inclination);
    float sin_inc = std::sin(orbit.inclination);

    // i need to access the trajectory of the parent SOI
    return Vector3N(
      o.x*(cos_ap*cos_lan - sin_ap*sin_lan*cos_inc) - o.y*(sin_ap*cos_lan + cos_ap*sin_lan*cos_inc),
      o.x*(cos_ap*sin_lan + sin_ap*cos_lan*cos_inc) + o.y*(cos_ap*cos_lan*cos_inc - sin_ap*sin_lan),
      o.x*sin_ap*sin_inc + o.y*cos_ap*sin_inc
    ) + trajectoryPosition(soiTrajectory(orbit.soi), time);
  }

  Vector3N orbitVelocity(const KeplerianOrbit& orbit, double orbitStart, double time) {
    // just return zero, orbiting around nothing makes no sense
    if (orbit.soi == entt::null) {
      return Vector3N(0, 0, 0);
    }

    double gravitationalParameter = soiGravitationalParameter(orbit.soi);

    double t = time - (orbitStart - orbit.timeFromPeriapsis);
    
    float M = meanAnomaly(orbit.eccentricity, orbit.semiLatusRectum, t, gravitationalParameter);
    float theta = trueAnomaly(M, orbit.eccentricity);
    float mu = gravitationalParameter;
    float r = orbit.semiLatusRectum / (1 + orbit.eccentricity * std::cos(theta));

    float h = sqrt(mu * orbit.semiLatusRectum);
    float v_r = (mu / h) * orbit.eccentricity * std::sin(theta);
    float v_theta = (mu / h) * (1 + orbit.eccentricity * std::cos(theta));
    Vector3N o = Vector3N(v_r * std::cos(theta) - v_theta * std::sin(theta), v_r * std::sin(theta) + v_theta * std::cos(theta), 0.0);

    float cos_ap = std::cos(orbit.argumentPeriapsis);
    float sin_ap = std::sin(orbit.argumentPeriapsis);
    float cos_lan = std::cos(orbit.longAscendingNode);
    float sin_lan = std::sin(orbit.longAscendingNode);
    float cos_inc = std::cos(orbit.inclination);
    float sin_inc = std::sin(orbit.inclination);

    return Vector3N(
      o.x*(cos_ap*cos_lan - sin_ap*sin_lan*cos_inc) - o.y*(sin_ap*cos_lan + cos_ap*sin_lan*cos_inc),
      o.x*(cos_ap*sin_lan + sin_ap*cos_lan*cos_inc) + o.y*(cos_ap*cos_lan*cos_inc - sin_ap*sin_lan),
      o.x*sin_ap*sin_inc + o.y*cos_ap*sin_inc
    ) + trajectoryVelocity(soiTrajectory(orbit.soi), time);
  }

  Vector3N trajectoryPosition(Trajectory trajectory, double time) {
    // get the orbit with the highest start time, that has already started at time

    if (trajectory.size() == 0) { return Vector3N(0, 0, 0); }

    const OrbitVariant* orbit = nullptr;
    double orbitStart = -INFINITY;

    for (const auto& [start, orbitVariant] : trajectory) {
      if (start <= time && start >= orbitStart) {
        orbit = &orbitVariant;
        orbitStart = start;
      }
    }

    if (!orbit) { return Vector3N(0, 0, 0); }

    return orbitPosition(std::get<KeplerianOrbit>(*orbit), orbitStart, time);
  }

  Vector3N trajectoryVelocity(Trajectory trajectory, double time) {
    if (trajectory.size() == 0) { return Vector3N(0, 0, 0); }

    const OrbitVariant* orbit = nullptr;
    double orbitStart = -INFINITY;

    for (const auto& [start, orbitVariant] : trajectory) {
      if (start <= time && start >= orbitStart) {
        orbit = &orbitVariant;
        orbitStart = start;
      }
    }

    if (!orbit) { return Vector3N(0, 0, 0); }

    return orbitVelocity(std::get<KeplerianOrbit>(*orbit), orbitStart, time);
  }

  KeplerianOrbit orbitFromCartesian(Vector3N position, Vector3N velocity, double start, Entity soi) {
    double gravitationalParameter = soiGravitationalParameter(soi);

    Vector3N h = position.cross(velocity);
    Vector3N eccV = velocity.cross(h) / gravitationalParameter - position.normalized();
    double ecc = eccV.magnitude();
    Vector3N n = Vector3N(-h.y, h.x, 0);

    double incl = acos(h.z / h.magnitude());

    double longAscNode = 0;
    double argPeri = 0;
    double timeFromPeri = 0;

    if (n.magnitude() < 1e-8 * h.magnitude()) {
      longAscNode = 0;
      if (ecc < 0.01) {
        argPeri = 0;
      }
      else {
        if (h.z >= 0) {
          argPeri = atan2(eccV.y, eccV.x);
        }
        else {
          argPeri = 2 * Constants::pi - atan2(eccV.y, eccV.x);
        }
      }
    }
    else {
      if (n.y >= 0) {
        longAscNode = acos(n.x / n.magnitude());
      }
      else {
        longAscNode = 2 * Constants::pi - acos(n.x / n.magnitude());
      }
      if (ecc < 0.01) {
        argPeri = 0;
      }
      else {
        if (eccV.z >= 0) {
          argPeri = acos(n.dot(eccV) / (ecc * n.magnitude()));
        }
        else {
          argPeri = 2 * Constants::pi - acos(n.dot(eccV) / (ecc * n.magnitude()));
        }
      }
    }

    double semiMajorAxis = 1 / ((2 / position.magnitude()) - (pow(velocity.magnitude(), 2) / gravitationalParameter));
    double semiLatusRectum = pow(h.magnitude(), 2) / gravitationalParameter;

    double theta;
    if (position.dot(velocity) >= 0) {
      theta = acos(eccV.dot(position) / (ecc * position.magnitude()));
    }
    else {
      theta = 2 * Constants::pi - acos(eccV.dot(position) / (ecc * position.magnitude()));
    }

    if (ecc < 1) {
      double E = 2 * atan(tan(theta / 2) / sqrt((1 + ecc) / (1 - ecc)));
      double M = E - ecc * sin(E);
      timeFromPeri = M / sqrt(gravitationalParameter / pow(semiMajorAxis, 3));
    }
    else if (ecc == 1) {
      double D = sqrt(2) * tan(theta / 2);
      double M = D + (pow(D, 3) / 3);
      timeFromPeri = M / sqrt(gravitationalParameter / pow(semiLatusRectum, 3));
    }
    else {
      double F = 2 * atanh(sqrt((ecc - 1) / (ecc + 1)) * tan(theta / 2));
      double M = ecc * sinh(F) - F;
      timeFromPeri = M / sqrt(gravitationalParameter / pow(abs(semiMajorAxis), 3));
    }

    return KeplerianOrbit{ecc, semiLatusRectum, incl, longAscNode, argPeri, timeFromPeri, soi};
  }

  std::vector<Entity> getSoiEntities() {
    std::vector<Entity> sois;
    auto view = context->registry->view<Orbiter, CelestialBody>();
    view.each([&](auto entity, auto &orbiter, auto &celestialBody){
      sois.push_back(entity);
    });
    return sois;
  }

  Entity getSoi(Vector3N position, float time) {
    // this function may be totally broken
    Entity newSoi = entt::null;
    double smallestRadius = INFINITY;

    for (auto& soi : getSoiEntities()) {
      //sphereOfInfluenceRadius = a * pow(m / M, 2/5);
      double soiRadius = getSoiRadius(soi);
      double distance = (position - trajectoryPosition(soiTrajectory(soi), time)).magnitude();

      if (soiRadius < smallestRadius && distance <= soiRadius) {
        smallestRadius = soiRadius;
        newSoi = soi;
      }
    }

    return newSoi;
  }

  std::optional<double> soiHandoverTime(KeplerianOrbit orbit, double orbitStart) {
    float step = 5e4;
    float orbitEnd = 0;
    float gravitationalParameter = soiGravitationalParameter(orbit.soi);

    if (orbit.eccentricity < 1) {
      float a = orbit.semiLatusRectum / (1 - pow(orbit.eccentricity, 2));
      float period = 2 * Constants::pi * sqrt(pow(a, 3) / gravitationalParameter);
      orbitEnd = orbitStart + period;
    }
    else {
      orbitEnd = orbitStart + 1e6;
    }

    Entity current = getSoi(orbitPosition(orbit, orbitStart, orbitStart), orbitStart);
    float prevTime = orbitStart;
    float t = orbitStart + step;

    while (t <= orbitEnd) {
      Entity soiNow = getSoi(orbitPosition(orbit, orbitStart, t), t);
      if (soiNow != current) {
        float lo = prevTime;
        float hi = t;
        while (hi - lo > 1e-3) {
          float mid = 0.5 * (lo + hi);
          if (getSoi(orbitPosition(orbit, orbitStart, mid), mid) == current) {
            lo = mid;
          }
          else {
            hi = mid;
          }
        }
        printf("hi: %f", hi);
        return hi;
      }
      prevTime = t;
      t += step;
    }

    return std::nullopt;
  }

  Trajectory extendTrajectory(Trajectory trajectory) {
    const int maxOrbits = 20;
    while (maxOrbits - trajectory.size() > 0) {
      auto lastOrbit = trajectory.back();
      std::optional<double> encounterTimeOpt = soiHandoverTime(std::get<KeplerianOrbit>(lastOrbit.second), lastOrbit.first);
      if (!encounterTimeOpt) {
        break;
      }
      // add one to ensure that we are actually in the new SOI.
      // this is probably not actually needed...
      double encounterTime = encounterTimeOpt.value() + 1;

      Entity encounterSoi = getSoi(orbitPosition(std::get<KeplerianOrbit>(lastOrbit.second), lastOrbit.first, encounterTime), encounterTime);
      if (encounterSoi == entt::null) {
        printf("Could not get encounter SOI, this is weird");
        return trajectory;
      }

      auto soiT = soiTrajectory(encounterSoi);

      Vector3N soiPos = trajectoryPosition(soiT, encounterTime);
      Vector3N soiVel = trajectoryVelocity(soiT, encounterTime);

      const KeplerianOrbit& orbit = std::get<KeplerianOrbit>(lastOrbit.second);
      Vector3N pos = orbitPosition(orbit, lastOrbit.first, encounterTime);
      Vector3N vel = orbitVelocity(orbit, lastOrbit.first, encounterTime);

      KeplerianOrbit newOrbit = orbitFromCartesian(pos - soiPos, vel - soiVel, encounterTime, encounterSoi);
      trajectory.push_back({encounterTime, newOrbit});
    }

    return trajectory;
  }

  void init() {
    auto stateView = context->registry->view<InitialOrbiterState, Orbiter, Spatial>();
    stateView.each([this](auto &initialState, auto &orbiter, auto &spatial){
      auto soi = getSoi(spatial.position, context->simTime);
      if (soi == entt::null) {
        return;
      }
      KeplerianOrbit orbit = orbitFromCartesian(spatial.position, initialState.velocity, context->simTime, soi); // the issue is here
      orbiter.trajectory.push_back({context->simTime, orbit});
      orbiter.trajectory = extendTrajectory(orbiter.trajectory);
    });
  }

  void update() {
    // SOIs are determined by celestial body objects
    // sphereOfInfluenceRadius = a * pow(m / M, 2/5);

    // if an orbiter's trajectory is null, then it's orbit
    // should be determined with the dummy Position and Velocity
    // components of the struct.
    // the primary purpose of this is to make it easy to create
    // and orbit without having to think about a trajectory trajectory

    auto orbiterView = context->registry->view<Orbiter, Spatial>();
    orbiterView.each([this](auto &orbiter, auto &spatial){
      Vector3N pos = trajectoryPosition(orbiter.trajectory, context->simTime);
      spatial.position = Vector3N(pos.x, pos.z, -pos.y);
      // potentially use orbiter.trajectory.velocity(context->simTime);
      // in order to interpolate between frames
    });
  }
};
