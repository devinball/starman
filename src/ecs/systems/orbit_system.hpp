#pragma once

#include "ecs/system.hpp"
#include "ecs/components/orbiter.hpp"
#include "ecs/components/spatial.hpp"
#include "ecs/components/tag.hpp"

#include "imgui.h"

struct OrbitSystem : System {
  double trajectoryEndTime(Trajectory trajectory) {
    double t = 0;
    for (auto [end, orbit] : trajectory) {
      if (end > t) { t = end; } 
    }
    return t;
  }

  Trajectory getSoiTrajectory(Entity soi) {
    return context->registry->get<Orbiter>(soi).trajectory;
  }

  double soiGravitationalParameter(Entity soi) {
    return context->registry->get<CelestialBody>(soi).gravitationalParameter;
  }

  double getSoiRadius(Entity soi) {
    return context->registry->get<CelestialBody>(soi).soiRadius;
  }

  double aToP(double a, double e) {
    return a * (1 - e*e);
  }

  double hyperbolicEccentricAnomaly(double M, double e) {
    double E = M; // initial guess for M
    
    auto kepler = [e](double E){ return e * std::sinh(E) - E; };

    double tolerance = 1e-10;
    int maxIterations = 1000;
    int iterations = 0;
    while (abs(kepler(E) - M) > tolerance && iterations < maxIterations) {
      E -= (kepler(E) - M) / (e * cosh(E) - 1);
      iterations += 1;
    }
    return E;
  }

  double elipticalEccentricAnomaly(double M, double e) {
    double E = M;

    auto kepler = [e](double E){ return E - e * std::sin(E); };

    double tolerance = 1e-10;
    int maxIterations = 1000;
    int iterations = 0;
    while (abs(kepler(E) - M) > tolerance && iterations < maxIterations) {
      E -= (kepler(E) - M) / (1 - e * cos(E));
      iterations += 1;
    }
    return E;
  }

  double elipticalTrueAnomaly(double M, double e) {
    double E = elipticalEccentricAnomaly(M, e);
    return 2 * atan(sqrt((1 + e) / (1 - e)) * tan(E / 2));
  }

  double parabolicTrueAnomaly(double M) {
    double D = cbrt(3 * M + sqrt(9 * pow(M, 2) + 2)) + cbrt(3 * M - sqrt(9 * pow(M, 2) + 1));
    return 2 * atan(D);
  }

  double hyperbolicTrueAnomaly(double M, double e) {
    double E = hyperbolicEccentricAnomaly(M, e);
    return 2 * atan(sqrt((e + 1) / (e - 1)) * tanh(E / 2));
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
      M = sqrt(gravitationalParameter / pow(abs(a), 3)) * time;
    }

    return M;
  }

  double trueAnomaly(double M, double e) {
    double theta;
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

  Vector3N orbitPosition(const KeplerianOrbit& orbit, double time) {
    // just return zero, orbiting around nothing makes no sense
    if (orbit.soi == entt::null) {
      return Vector3N(0, 0, 0);
    }

    double gravitationalParameter = soiGravitationalParameter(orbit.soi);

    double t = time - orbit.timeOfPeriapsis;

    double M = meanAnomaly(orbit.eccentricity, orbit.semiLatusRectum, t, gravitationalParameter);
    double theta = trueAnomaly(M, orbit.eccentricity);
    double r = orbit.semiLatusRectum / (1 + orbit.eccentricity * std::cos(theta));
    Vector3N o = Vector3N(std::cos(theta), std::sin(theta), 0) * r;

    double cos_ap = std::cos(orbit.argumentPeriapsis);
    double sin_ap = std::sin(orbit.argumentPeriapsis);
    double cos_lan = std::cos(orbit.longAscendingNode);
    double sin_lan = std::sin(orbit.longAscendingNode);
    double cos_inc = std::cos(orbit.inclination);
    double sin_inc = std::sin(orbit.inclination);

    // i need to access the trajectory of the parent SOI
    return Vector3N(
      o.x*(cos_ap*cos_lan - sin_ap*sin_lan*cos_inc) - o.y*(sin_ap*cos_lan + cos_ap*sin_lan*cos_inc),
      o.x*(cos_ap*sin_lan + sin_ap*cos_lan*cos_inc) + o.y*(cos_ap*cos_lan*cos_inc - sin_ap*sin_lan),
      o.x*sin_ap*sin_inc + o.y*cos_ap*sin_inc
    ) + trajectoryPosition(getSoiTrajectory(orbit.soi), time);
  }

  Vector3N orbitVelocity(const KeplerianOrbit& orbit, double time) {
    // just return zero, orbiting around nothing makes no sense
    if (orbit.soi == entt::null) {
      return Vector3N(0, 0, 0);
    }

    double gravitationalParameter = soiGravitationalParameter(orbit.soi);

    double t = time - orbit.timeOfPeriapsis;
    
    double M = meanAnomaly(orbit.eccentricity, orbit.semiLatusRectum, t, gravitationalParameter);
    double theta = trueAnomaly(M, orbit.eccentricity);
    double mu = gravitationalParameter;
    double r = orbit.semiLatusRectum / (1 + orbit.eccentricity * std::cos(theta));

    double h = sqrt(mu * orbit.semiLatusRectum);
    double v_r = (mu / h) * orbit.eccentricity * std::sin(theta);
    double v_theta = (mu / h) * (1 + orbit.eccentricity * std::cos(theta));
    Vector3N o = Vector3N(v_r * std::cos(theta) - v_theta * std::sin(theta), v_r * std::sin(theta) + v_theta * std::cos(theta), 0.0);

    double cos_ap = std::cos(orbit.argumentPeriapsis);
    double sin_ap = std::sin(orbit.argumentPeriapsis);
    double cos_lan = std::cos(orbit.longAscendingNode);
    double sin_lan = std::sin(orbit.longAscendingNode);
    double cos_inc = std::cos(orbit.inclination);
    double sin_inc = std::sin(orbit.inclination);

    return Vector3N(
      o.x*(cos_ap*cos_lan - sin_ap*sin_lan*cos_inc) - o.y*(sin_ap*cos_lan + cos_ap*sin_lan*cos_inc),
      o.x*(cos_ap*sin_lan + sin_ap*cos_lan*cos_inc) + o.y*(cos_ap*cos_lan*cos_inc - sin_ap*sin_lan),
      o.x*sin_ap*sin_inc + o.y*cos_ap*sin_inc
    ) + trajectoryVelocity(getSoiTrajectory(orbit.soi), time);
  }

  const OrbitVariant* activeSegment(const Trajectory& trajectory, double time) {
    // get the orbit with the smallest end time, that still remains in the future
    if (trajectory.size() == 0) { return nullptr; }
    const OrbitVariant* best = nullptr;
    double bestEnd = 0;
    for (const auto& [end, variant] : trajectory) {
      if (end >= time && (best == nullptr || end < bestEnd)) {
        best = &variant;
        bestEnd = end;
      }
    }
    return best;
  }

  Vector3N trajectoryPosition(Trajectory trajectory, double time) {
    const OrbitVariant* orbit = activeSegment(trajectory, time);
    return orbit ? orbitPosition(std::get<KeplerianOrbit>(*orbit), time) : Vector3N(0, 0, 0);
  }

  Vector3N trajectoryVelocity(Trajectory trajectory, double time) {
    const OrbitVariant* orbit = activeSegment(trajectory, time);
    return orbit ? orbitVelocity(std::get<KeplerianOrbit>(*orbit), time) : Vector3N(0, 0, 0);
  }

  KeplerianOrbit orbitFromCartesian(Vector3N position, Vector3N velocity, double currentTime, Entity soi) {
    double gravitationalParameter = soiGravitationalParameter(soi);

    Vector3N h = position.cross(velocity);
    Vector3N eccV = velocity.cross(h) / gravitationalParameter - position.normalized();
    double ecc = eccV.magnitude();
    Vector3N n = Vector3N(-h.y, h.x, 0);

    double incl = acos(h.z / h.magnitude());

    double longAscNode = 0;
    double argPeri = 0;
    double timeOfPeri = 0;

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

    // timeFromPeri is wrong way to think about this
    // instead timeOfPeri

    if (ecc < 1) {
      //double E = 2 * atan(tan(theta / 2) / sqrt((1 + ecc) / (1 - ecc)));
      double E = 2 * atan2(sqrt(1 - ecc) * sin(theta / 2), sqrt(1 + ecc) * cos(theta/2));
      double M = E - ecc * sin(E);
      timeOfPeri = currentTime - M / sqrt(gravitationalParameter / pow(semiMajorAxis, 3));
    }
    else if (ecc == 1) { //todo: check this case for math problems
      double D = tan(theta / 2);
      double M = D + (pow(D, 3) / 3);
      timeOfPeri = currentTime - 0.5 * sqrt(pow(semiLatusRectum, 3) / gravitationalParameter) * M;
    }
    else {
      double F = 2 * atanh(sqrt((ecc - 1) / (ecc + 1)) * tan(theta / 2));
      double M = ecc * sinh(F) - F;
      timeOfPeri = currentTime - M / sqrt(gravitationalParameter / pow(abs(semiMajorAxis), 3));
    }

    return KeplerianOrbit{ecc, semiLatusRectum, incl, longAscNode, argPeri, timeOfPeri, soi};
  }

  std::vector<Entity> getSoiEntities() {
    std::vector<Entity> sois;
    auto view = context->registry->view<Orbiter, CelestialBody>();
    view.each([&](auto entity, auto &orbiter, auto &celestialBody){
      sois.push_back(entity);
    });
    return sois;
  }

  Entity getSoi(Vector3N position, double time) {
    // stars will need to be a little different
    // instead of doing within a radius, you just
    // use the closest star.

    // this is going to be wonky with an SOI
    // / orbiter, because it's always
    // it it's own SOI, so i need to exclude
    // those cases from being in their own SOIs

    Entity newSoi = entt::null;
    double smallestRadius = INFINITY;

    for (auto& soi : getSoiEntities()) {
      //sphereOfInfluenceRadius = a * pow(m / M, 2/5);
      double soiRadius = getSoiRadius(soi);
      double distance = (position - trajectoryPosition(getSoiTrajectory(soi), time)).magnitude();

      if (soiRadius < smallestRadius && distance <= soiRadius) {
        smallestRadius = soiRadius;
        newSoi = soi;
      }
    }

    return newSoi;
  }

  double calculateOrbitalPeriod(KeplerianOrbit orbit) {
    double gravitationalParameter = soiGravitationalParameter(orbit.soi);
    double semiMajorAxis = orbit.semiLatusRectum / (1 - pow(orbit.eccentricity, 2));
    return 2 * Constants::pi * sqrt(pow(semiMajorAxis, 3) / gravitationalParameter);
  }

  double soiHandoverTime(KeplerianOrbit orbit, double currentTime) {
    // TODO:
    // more informed step size and orbit length
    // actually do the nearby optimization

    const double maxOrbitLength = 1e7;
    double time = currentTime;
    double step = 5e4;

    if (currentTime == INFINITY) {
      printf("soiHandoverTime called infinite current time, this is unusual\n");
      return maxOrbitLength;
    }

    if (orbit.soi == entt::null) {
      printf("soiHandoverTime called on orbit with null SOI, this is unusual\n");
      return maxOrbitLength;
    }

    Entity startingSoi = getSoi(orbitPosition(orbit, time), time);
    
    while (time <= currentTime + maxOrbitLength) {
      Vector3N position = orbitPosition(orbit, time);
      Entity currentSoi = getSoi(position, time);

      if (currentSoi != startingSoi) {
        printf("!!!!! %f\n", time);
        return time;
        // optimize to get better estimate
        // it must be between currentTime and currentTime - step;
      }

      time += step;
    }

    return currentTime + maxOrbitLength;
  }

  Trajectory extendTrajectory(Trajectory trajectory) {
    // assume we start with a single Segment
    // out goal is to extend that segment arbitrarily
    // far into the future

    const int maxOrbits = 5;

    // wait, this function relies on the orbits already having proper end times
    // but they might not always?

    while (trajectory.size() < maxOrbits) {
      auto [lastOrbitEndTime, lastOrbit] = trajectory.back();

      Vector3N position = trajectoryPosition(trajectory, lastOrbitEndTime);
      Vector3N velocity = trajectoryVelocity(trajectory, lastOrbitEndTime);

      auto soi = getSoi(position, lastOrbitEndTime);
    
      if (soi == entt::null) { return trajectory; }

      Vector3N soiPosition = trajectoryPosition(getSoiTrajectory(soi), lastOrbitEndTime);
      Vector3N soiVelocity = trajectoryVelocity(getSoiTrajectory(soi), lastOrbitEndTime);

      KeplerianOrbit newOrbit = orbitFromCartesian(position - soiPosition, velocity - soiVelocity, lastOrbitEndTime, soi);

      double encounterTime = soiHandoverTime(newOrbit, lastOrbitEndTime);
    
      trajectory.push_back({encounterTime, newOrbit});
    }
    
    return trajectory;

    /*
    while (trajectory.size() < maxOrbits) {
      auto lastOrbit = trajectory.back(); // should probably actually find the latest trajectory (time wise)

      // start looking for the next orbit when the trajectory ends
      double encounterTime = soiHandoverTime(std::get<KeplerianOrbit>(lastOrbit.second), lastOrbit.first);

      Entity encounterSoi = getSoi(orbitPosition(std::get<KeplerianOrbit>(lastOrbit.second), encounterTime), encounterTime);
      if (encounterSoi == entt::null) {
        printf("Could not get encounter SOI, this is weird\n");
        return trajectory;
      }

      printf("Got encounter soi time %f\n", encounterTime);

      auto soiT = getSoiTrajectory(encounterSoi);

      Vector3N soiPos = trajectoryPosition(soiT, encounterTime);
      Vector3N soiVel = trajectoryVelocity(soiT, encounterTime);

      const KeplerianOrbit& orbit = std::get<KeplerianOrbit>(lastOrbit.second);
      Vector3N pos = orbitPosition(orbit, encounterTime);
      Vector3N vel = orbitVelocity(orbit, encounterTime);

      KeplerianOrbit newOrbit = orbitFromCartesian(pos - soiPos, vel - soiVel, encounterTime, encounterSoi);
      // here i need to put where the orbit should end.
      // this maybe needs to be recursive?
      // a Segments end should be informed by when that segment
      // switches SOI, preforms a manuver, or other force applied
      trajectory.push_back({encounterTime, newOrbit});
    }

    return trajectory;
    */
  }

  Trajectory purgeTrajectory(Trajectory trajectory, double currentTime) {
    Trajectory newTrajectory;
    Segment mostRecent = trajectory.back();
    double mostRecentTime;
    for (Segment segment : trajectory) {
      if (segment.first >= currentTime) {
        newTrajectory.push_back(segment);
      }
      else if (segment.first > mostRecentTime) {
        mostRecent = segment;
      }
    }

    if (newTrajectory.size() == 0) { 
      newTrajectory.push_back(mostRecent);
    }

    return newTrajectory;
  }

  // ok, sometimes orbiters are just disappearing, i think when they go fast
  // or are really far?
  // i suspect there is some sort of NAN/infinity going on here, or a giant discontinuity
  // in position.
  void init() {
    auto stateView = context->registry->view<InitialOrbiterState, Orbiter, Spatial>();
    stateView.each([this](auto &initialState, auto &orbiter, auto &spatial){
      auto soi = getSoi(spatial.position, context->simTime);

      if (soi == entt::null) { return; }

      auto soiT = getSoiTrajectory(soi);

      Vector3N soiPos = trajectoryPosition(soiT, context->simTime);
      Vector3N soiVel = trajectoryVelocity(soiT, context->simTime);

      KeplerianOrbit orbit = orbitFromCartesian(spatial.position - soiPos, initialState.velocity - soiVel, context->simTime, soi);

      //printf("%f\n", orbit.semiLatusRectum);

      // should not be infinity...
      // but trajectory extend should work?
      orbiter.trajectory.push_back({context->simTime + 1, orbit});
      orbiter.trajectory = extendTrajectory(orbiter.trajectory);

      //printf("Tlen: %f\n", double(orbiter.trajectory.size()));

      //Vector3N t = trajectoryPosition(orbiter.trajectory, context->simTime + 10);
      //printf("POS %f %f %f\n", t.x, t.y, t.z);

      //Vector3N t2 = orbitPosition(orbit, context->simTime);
      //printf("POS2 %f %f %f\n", double(t2.x), double(t2.y), double(t2.z));

      //printf("%f %f %f\n", trajectoryPosition(orbiter.trajectory, context->simTime));
    });
  }

  // remaining things to do:
  // 1. barycenters and binary systems
  //      i'm not sure how exactly to do this
  //      in orbit position i get the trajectory
  //      position of the parent. maybe this is
  //      totally the wrong way to do this?
  // 2. code cleanup.
  //      having to query the ECS probably isn't
  //      ideal? maybe this would help with the
  //      barycenter thing as well
  //      of course having two world states would
  //      be much worse, as is it's pretty easy
  //      to set everything up, which is nice
  // 3. encounters don't seem to be working?
  //      need to verify that encounter system
  //      makes physical sense
  // 4. discontinuity watchdog.
  //      more physics/math issues will likely
  //      occur, so having something that makes
  //      sure that everything is (mostly) continuous
  //      is probably a good idea.

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
      if (trajectoryEndTime(orbiter.trajectory) <= context->simTime) {
        //printf("end times\n");
        orbiter.trajectory = purgeTrajectory(orbiter.trajectory, context->simTime);
        orbiter.trajectory = extendTrajectory(orbiter.trajectory);
      }
      Vector3N pos = trajectoryPosition(orbiter.trajectory, context->simTime);
      //printf("POS: %f %f %f\n", pos.x, pos.y, pos.z);
      // position is indeed nan, nan, nan or the additive inverse thereof
      spatial.position = Vector3N(pos.x, pos.z, -pos.y);
      // potentially use orbiter.trajectory.velocity(context->simTime);
      // in order to interpolate between physics frames
    });
  }
};
