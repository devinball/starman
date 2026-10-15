#pragma once

#include "ecs/system.hpp"
#include "ecs/components/orbiter.hpp"
#include "ecs/components/spatial.hpp"
#include "ecs/components/tag.hpp"

#include "imgui.h"

struct OrbitSystem : System {
  double acos_clamped(double x) {
    return std::acos(std::clamp(-1.0, 1.0, x));
  }

  double trajectoryEndTime(const Trajectory& trajectory) {
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

  /*
  double hyperbolicEccentricAnomaly(double M, double e) {
    double E = M; // initial guess for M
    
    auto kepler = [e](double E){ return e * std::sinh(E) - E; };

    double tolerance = 1e-10;
    int maxIterations = 1000;
    int iterations = 0;
    while (std::abs(kepler(E) - M) > tolerance && iterations < maxIterations) {
      E -= (kepler(E) - M) / (e * cosh(E) - 1);
      iterations += 1;
    }

    if (std::isnan(e)) { printf("e\n"); }
    if (std::isnan(M)) { printf("M\n"); }
    if (std::isnan(E)) { printf("E\n"); }

    // its just E that is nan

    return E;
  }
  */

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
      M = sqrt(gravitationalParameter / pow(std::abs(a), 3)) * time;
    }

    return M;
  }

  double trueAnomaly(double M, double e) {
    double theta;
    if (e < 1) {
      theta = elipticalTrueAnomaly(M, e);
      if (std::isnan(theta)) { printf("theta e\n"); }
    }
    else if (e == 1) {
      theta = parabolicTrueAnomaly(M);
      if (std::isnan(theta)) { printf("theta p\n"); }
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
    double v_theta = (mu / h) * (1 + orbit.eccentricity * std::cos(theta)); // TODO issue above here, if h is 0, the issue could occur
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

  Vector3N orbitPosition(const KinematicOrbit& orbit, double time) {
    return orbit.position + orbit.velocity * (time - orbit.initialTime) + orbit.acceleration * 0.5 * pow(time - orbit.initialTime, 2);
  }

  Vector3N orbitVelocity(const KinematicOrbit& orbit, double time) {
    return orbit.velocity + orbit.acceleration * (time - orbit.initialTime);
  }

  const OrbitVariant* activeSegment(const Trajectory& trajectory, double time) {
    // get the orbit with the smallest end time, that still remains in the future
    if (trajectory.size() == 0) { printf("empty trajectory!\n"); return nullptr; }
    const OrbitVariant* best = nullptr;
    const OrbitVariant* last = nullptr;
    double bestEnd = -INFINITY;
    double lastEnd = -INFINITY;
    for (const auto& [end, variant] : trajectory) {
      if (end >= time && (!best || end < bestEnd)) {
        best = &variant;
        bestEnd = end;
      }
      if (end > lastEnd) {
        last = &variant;
        lastEnd = end;
      }
    }
    // return best if it exists, otherfallback to the last orbit
    return best ? best : last;
  }

  Vector3N trajectoryPosition(const Trajectory& trajectory, double time) {
    const OrbitVariant* orbit = activeSegment(trajectory, time);
    return orbit ? std::visit([this, time](const auto& i) { return orbitPosition(i, time); }, *orbit) : Vector3N(0, 0, 0);
  }

  Vector3N trajectoryVelocity(const Trajectory& trajectory, double time) {
    const OrbitVariant* orbit = activeSegment(trajectory, time);
    return orbit ? std::visit([this, time](const auto& i) { return orbitVelocity(i, time); }, *orbit) : Vector3N(0, 0, 0);
  }

  KeplerianOrbit orbitFromCartesian(Vector3N position, Vector3N velocity, double currentTime, Entity soi) {
    double gravitationalParameter = soiGravitationalParameter(soi);

    Vector3N h = position.cross(velocity);
    Vector3N eccV = velocity.cross(h) / gravitationalParameter - position.normalized();
    double ecc = eccV.magnitude();
    Vector3N n = Vector3N(-h.y, h.x, 0);

    double incl = acos_clamped(h.z / h.magnitude());

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
        longAscNode = acos_clamped(n.x / n.magnitude());
      }
      else {
        longAscNode = 2 * Constants::pi - acos_clamped(n.x / n.magnitude());
      }
      if (ecc < 1e-6) {
        argPeri = 0;
      }
      else {
        if (eccV.z >= 0) {
          argPeri = acos_clamped(n.dot(eccV) / (ecc * n.magnitude()));
        }
        else {
          argPeri = 2 * Constants::pi - acos_clamped(n.dot(eccV) / (ecc * n.magnitude()));
        }
      }
    }

    double semiMajorAxis = 1 / ((2 / position.magnitude()) - (pow(velocity.magnitude(), 2) / gravitationalParameter));
    double semiLatusRectum = pow(h.magnitude(), 2) / gravitationalParameter;

    double theta;
    if (position.dot(velocity) >= 0) {
      theta = acos_clamped(eccV.dot(position) / (ecc * position.magnitude()));
    }
    else {
      theta = 2 * Constants::pi - acos_clamped(eccV.dot(position) / (ecc * position.magnitude()));
    }

    if (ecc < 1) {
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
      timeOfPeri = currentTime - M / sqrt(gravitationalParameter / pow(std::abs(semiMajorAxis), 3));
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
    Entity newSoi = entt::null;
    double smallestRadius = INFINITY;

    for (auto& soi : getSoiEntities()) {
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

  bool stableOrbit(KeplerianOrbit orbit, double currentTime) {
    //if 

    return false;
  }

  std::pair<double, bool> soiHandoverTime(KeplerianOrbit orbit, double currentTime) {
    // TODO: more informed step size and orbit length

    
    const double maxOrbitLength = 1e8;
    const double step = 5e4;

    /*
    Vector3N nextPosition = orbitPosition(orbit, time);
    Vector3N lastPosition = orbitPosition(orbit, time);

    double distance = (nextPosition - lastPosition).magnitude();

    Vector3N velocity = orbitVelocity(orbit, time);
    double timeStep =  / velocity.magnitude();
    // time = distance / speed

    // i need the straight line time/distance between the spacecraft and the SOI

    // i could also use orbitVelocity to find the distance for a given time

    // biggest step should be distance, in fact, stepping by distance should be 
    // good enought. i need to step in time, not in distances

    for (double time = currentTime + step)
    */

    for (double time = currentTime + step; time <= currentTime + maxOrbitLength; time += step) {
      if (orbit.soi != getSoi(orbitPosition(orbit, time), time)) {
        double high = time;
        double low = time - step;

        while (high - low > 1) {
          double mid = 0.5 * (high + low);
          if (orbit.soi != getSoi(orbitPosition(orbit, mid), mid)) { high = mid; }
          else { low = mid; }
        }
        return std::make_pair(high, true);
      }
    }

    return std::make_pair(currentTime + maxOrbitLength, false);
  }

  // TODO!! very important. there seems to be a bug, where eventually
  // orbiters just disappear, i obsersved this happen to many orbiters
  // at a similar distance from the sun, however i don't think its directly
  // a distance issue, because some orbiters made it way past that point.
  // perhaps it is a time issue? i thought maybe some were being teleported
  // to 0, 0, 0, but i'm not sure
  void extendTrajectory(Trajectory& trajectory) {
    // this is assuming that the back of the trajectory is always the most
    // recent, which is probably a fine assumption to make.
    auto [lastOrbitEndTime, lastOrbitVariant] = trajectory.back();

    Vector3N position = trajectoryPosition(trajectory, lastOrbitEndTime);
    Vector3N velocity = trajectoryVelocity(trajectory, lastOrbitEndTime);


    // i suspect the issue actually arises in trajectory position/velocity
    // the nan causes it to not find the SOI, at which point kinematic
    // orbit doesn't matter, because it's all nan as well

    Entity newSoi = getSoi(position, lastOrbitEndTime);
    // maybe if no soi is found i should instead add a KinematicOrbit with 0 acceleration
    if (newSoi == entt::null) { 
      // this prevents objects from just disapearing when outside an SOI
      // infinity is temporary
      //printf("%f, %f, %f | %f, %f, %f\n", position.x, position.y, position.z, velocity.x, velocity.y, velocity.z);
      // a test particle created ~277 orbits while approaching soi boundary
      // i believe this to be the expected behavior after testing, it just actually
      // takes 277 * maxOrbitLength to leave the soi, at which point it's trajectory
      // becomes kinematic
      trajectory.push_back({INFINITY, KinematicOrbit{position, velocity, Vector3N(0, 0, 0), lastOrbitEndTime}});
      return;
    }

    Vector3N soiPosition = trajectoryPosition(getSoiTrajectory(newSoi), lastOrbitEndTime);
    Vector3N soiVelocity = trajectoryVelocity(getSoiTrajectory(newSoi), lastOrbitEndTime);

    orbitsUpdated += 1;

    KeplerianOrbit newOrbit = orbitFromCartesian(position - soiPosition, velocity - soiVelocity, lastOrbitEndTime, newSoi);

    auto [encounterTime, hasEncounter] = soiHandoverTime(newOrbit, lastOrbitEndTime);

    //printf("Segment length: %f\n", encounterTime - lastOrbitEndTime);

    trajectory.push_back({encounterTime, newOrbit});
  }

  void purgeTrajectory(Trajectory& trajectory, double currentTime) {
    // again assumes that trajectory is sorted oldest at front newest at back
    int idx = 0;
    for (Segment segment : trajectory) {
      if (segment.first > currentTime) {
        break;
      }
      idx += 1;
    }
    if (idx > 1) {
      trajectory.erase(trajectory.begin(), trajectory.begin() + idx - 1);
    }
    /*
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
    */
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
  // 3. physical accuracy
  //      need to verify that encounter system
  //      makes physical sense
  // 4. discontinuity watchdog.
  //      more physics/math issues will likely
  //      occur, so having something that makes
  //      sure that everything is (mostly) continuous
  //      is probably a good idea.
  //      sometimes orbiters seem to be disappearing,
  //      i suspect that this is a nan issue
  //      maybe not, it seems things are just teleporting
  //      sometimes. i am unsure of the cause.
  // 5. initial conditions and setup
  //      i need to system to better deal with setup
  //      or initial conditions in general. a 2 body newtonian
  //      orbit seems a natural way to do this.
  // 6. better star SOI partitioning
  //      will need to be binary star system ready
  // 7. prevent SOIs from seeing themselves
  // 8. calculate orbits across multiple frames
  //      anticipate the trajectories that need to be
  //      extended soonest, then begin working on them
  //      early, so instead hundreds of orbits in one
  //      physics frame every couple seconds, it does a
  //      couple of orbits every frame.
  // 9. multithreading should also be possible.
  //      because orbits can be mostly independant, each
  //      thread would need a copy of the SOIs, but that
  //      wouldn't be too bad with a couple threads each
  //      with many orbits to compute
  // 10. dynamically lower simSpeed when a lot of things
  //      need to be computed in order to keep decent
  //      framerate and not mess up physics rate.
  
  // at high speeds, its obvious that far outthings on hyperbolic orbits
  // are moving in some sort of circular motion as well?
  // maybe based on last soi they encountered???
  // maybe its

  float timeExp = 0;
  int orbitsUpdated = 0;

  void draw() {
    ImGui::SliderFloat("Time Exponent", &timeExp, 0.0f, std::log10(86400*365*10)); // max of 10 years per second
    ImGui::Text("Time Scale (second/second): %f", context->simSpeed);
    ImGui::Text("Time Scale (years/second): %f", context->simSpeed / (86400 * 365)); // 86400 * 365 = year/second

    ImGui::Text("Total Orbits Created: %d", orbitsUpdated);

    context->simSpeed = pow(10, timeExp);
  }

  void update() {
    auto orbiterView = context->registry->view<Orbiter, Spatial>();
    orbiterView.each([this](auto &orbiter, auto &spatial){
      // make sure that trajectories don't run out at this time, or at the time that the next update will be
      // this is important at really high sim speeds where the delta time may be longer than an orbit
      if (trajectoryEndTime(orbiter.trajectory) <= context->simTime + context->dt * context->simSpeed * 1.15) {
        extendTrajectory(orbiter.trajectory);
        purgeTrajectory(orbiter.trajectory, context->simTime);
      }
      Vector3N pos = trajectoryPosition(orbiter.trajectory, context->simTime);

      // i have to flip coordiantes here, this is because the orbit math uses a different system
      // to the renderer, yes i very much need to fix this
      spatial.position = Vector3N(pos.x, pos.z, -pos.y);
    });

    /*
    context->registry->view<Spatial, Tag>().each([this](auto &spatial, auto &tag){
      if (tag.name == "orbiter") {
        printf("%s: (%f, %f, %f)\n", tag.name.c_str(), spatial.position.x, spatial.position.y, spatial.position.z);
      }
    });
    */
  }
};
