#pragma once

#include "math/constants.hpp"
#include "math/vector.hpp"
#include "ecs/system.hpp"

#include "ecs/components/rigidbody.hpp"
#include "ecs/components/spatial.hpp"
#include "ecs/components/keplerian_orbiter.hpp"

#include "math/utilities.hpp"

#include <math.h>

struct PhysicsSystem : System {
  int timeExp = 0;
  double t = 0;

  double kepler(double E, double e) {
    return E - e * std::sin(E);
  }

  double eccentric_anomaly(double M, double e) {
    double E = M;
    //TODO: use guess instead for faster convergance
    
    double tolerance = 1e-10;
    for (int i = 0; i < 100; ++i) {
      double x = kepler(E, e) - M;
      if (std::abs(x) <= tolerance) {
        break;
      }
      E -= x / (1 - e * std::cos(E));
    }

    return E;
  }

  Vector3N keplerToCartesian(double E, double e, double a, double i, double w, double o) {      
    // a = semiMajorAxis
    // e = eccentiricty
    // i = inclination
    // l = mean longitude
    // u = longitude of periapsis
    // o = longitude of ascending node

    // https://ssd.jpl.nasa.gov/planets/approx_pos.html

    double r = a * (1 - e * std::cos(E));
    double b = e / (1 + std::sqrt(1 - e * e)); // beta, intermediate for finding true anomaly
    double f = E + 2 * std::atan((b * std::sin(E)) / (1 - b * std::cos(E))); // true anomaly, using R. Broucke and P. Cefola method

    Number x = r * (std::cos(o) * std::cos(w + f) - std::sin(o) * std::sin(w + f) * std::cos(i));
    Number y = r * (std::sin(o) * std::cos(w + f) + std::cos(o) * std::sin(w + f) * std::cos(i));
    Number z = r * std::sin(w + f) * std::sin(i);
    
    // coordinate schenagins
    return Vector3N(x, z, y);
  }

  Vector3N getPosition(KeplerianOrbiter keplerian, double time) {
    double M = 2 * std::numbers::pi * time / keplerian.orbitalPeriod;
    assert(keplerian.orbitalPeriod > 0);
    double E = eccentric_anomaly(M, keplerian.eccentricity);

    // TODO: orbits precess over time, account for this
    Vector3N pos = keplerToCartesian(
      E,
      keplerian.eccentricity,
      keplerian.semiMajorAxis,
      keplerian.inclination,
      keplerian.longAscendingNode,
      keplerian.longPeriapsis
    );

    if (context->registry->valid(keplerian.parent)) {
      keplerian.barycenter = context->registry->get<Spatial>(keplerian.parent).position;
    }

    return pos + keplerian.barycenter;
  }

  Vector3N calcGravitationalForce(Vector3N a, Vector3N b, float mass1, float mass2) {
    Vector3N dr = b - a;
    Number d2 = dr.squareMagnitude() + 1;  
    Number magnitude = 6.6743015e-11 * mass1 * mass2 / d2;
    return dr.normalized() * magnitude;
  }

// calc forces here
// maybe i just pass in a list of forces, which i then sum up here?
// but how does it know when all of those forces should be applied?
// maybe it could be like how the keplerians work? but that doesn't
// make a ton of sense for a rocket burn.
// ok, so normally i would model a rocket burn by 1. applying a force to the object
// and 2. reducing the mass of it (by the appropriate amount).
// maybe i keep a list of rocket burns, these contain the direction, the duration
// start time, end time, engine efficiency, mass flow rate, thrust etc.
// i probably need to add mass + moment inertia to state, so at this point state will
// just be what Rigidbody used to be. so i guess rigidbody just contains trajectroies,
    // as well as the current, interpolated, state.
  State integrateState(State& state, double dt) {
    State newState = state;
    newState.time += dt;

    newState.position = state.position + state.velocity * dt + state.acceleration * (dt * dt) * 0.5;

    Vector3N netForce;

    auto keplerians = context->registry->view<KeplerianOrbiter>();  

    keplerians.each([&](auto &keplerian){
      Vector3N pos = getPosition(keplerian, state.time);
      netForce += calcGravitationalForce(newState.position, pos, newState.mass, keplerian.mass);
    });

    newState.acceleration = netForce / newState.mass;
    newState.velocity = state.velocity + (state.acceleration + newState.acceleration) * (dt * 0.5);

    return newState;
  }

  State integrateIter(State state, double dt, int numSteps) {
    // collisions could just be a manuver with a short duration and 100% reliability
   // this could be run on a seperate thread, even a seperate thread for each trajectory
    for (int step = 0; step < numSteps; ++step) {
      // ensure that the dt passed into this is the one actually used
      state = integrateState(state, dt);
    }
    
    return state;
  }

  void init() {
    
    auto rigidbodies = context->registry->view<Rigidbody, Spatial>();

    rigidbodies.each([&](auto &rigidbody, auto &spatial){
      rigidbody.currentState = State(
        0, 1000, 1000,
        spatial.position, Vector3N{0, 0, 30e4}, Vector3N{0, 0, 0},
        spatial.rotation
      );
    });

    double dt = 0.1;
    rigidbodies.each([&](auto &rigidbody, auto &spatial){
      State state = rigidbody.currentState;
      for (int i = 0; i < 500; ++i) {
        //printf("%f %f %f\n", state.position.x, state.position.y, state.position.z);
        rigidbody.future.push(state);
        state = integrateIter(state, dt, 10000);
      }
    });

    // maybe what i do is just allow any function that has the right signature
    // so just some function r(double time) -> Vector3N
  }

  void draw() {
    // this allows me to interpolate, so the physics engine can
    // have it's own position kept precisely, and the renderer
    // can see interpolated motion.
    // we want to be able to set position via the spatial
    // rigidbody.position is physics internal only
    // it should probably detect if spatial.position has been
    // externaly changed, and update rigidbody.position to track
    ImGui::SliderInt("Time Exp", &timeExp, 0, 10/*3600 * 24 * 365*/);

    auto spatials = context->registry->view<Rigidbody, Spatial>();

    spatials.each([&](auto &rigidbody, auto &spatial){
      spatial.position = rigidbody.currentState.position;
      spatial.rotation = rigidbody.currentState.rotation;
    });

    auto keplerians = context->registry->view<KeplerianOrbiter, Spatial>();  

    keplerians.each([&](auto &keplerian, auto &spatial) {
      spatial.position = getPosition(keplerian, t);
    });
  }

  void update() {
    t += context->dt * pow(10, timeExp);

    auto rigidbodies = context->registry->view<Rigidbody>();
    auto keplerians = context->registry->view<KeplerianOrbiter, Spatial>();

    rigidbodies.each([this](auto &rigidbody){

      // the basic idea is that i need to update something's position here
      // basically spatial.position = rigidbody.currentState.position;
      // so right here maybe i update currentState and nextState
      // i interpolate between them in the draw() function
      // so first i need to determine the SOI
      // its possible that this approach may not work, in which
      // case it may be a good idea to have an SOI range (calculated or defined)
      // then it just chooses the smallest one that it is within

      CelestialBody dominant;
      double maxForce = 0;
      for (auto celestialBody : celestialBodies) {
        // it's okay to use square magnitude here because we are just comparing
        // positive values, and if a > b, a > 0, b > 0, then a^2 > b^2
        force = calcGravitationalForce().squareMagnitude();
        if (force > maxForce) {
          dominant = celestialBody;
        } 
      }

      // we then use the dominant body as our SOI, and calculate traditional kepler orbits
      // this does mean that our keplerian orbit function needs to work with ALL trajectories
      // this includse parabolic and hyperbolic.

      // i probably need to switch based on eccentricity.
      // e = 0 = circular
      // e < 1 = elipse
      // e = 1 = parabola
      // e > 1 = hyperbola


      
      /*
      rigidbody.angularMomentum += rigidbody.netTorque * context->dt;
      rigidbody.angularVelocity = rigidbody.angularMomentum * (1 / rigidbody.momentInertia); // TODO : multiply by inverse momentInteria matrix instead
      rigidbody.rotation += rigidbody.rotation * rigidbody.angularVelocity * 0.5f * context->dt;
      rigidbody.rotation = rigidbody.rotation.normalized();
      */

      //double energy = 0.5 * rigidbody.mass * rigidbody.velocity.magnitude() + 6743015e-11 * rigidbody.mass * 1989100e24 / rigidbody.position.magnitude();
      //printf("Total energy: %f EJ | Speed: %f KM/S | Distance: %f MM \n", energy * 1e-18, rigidbody.velocity.magnitude() * 1e-3, rigidbody.position.magnitude() * 1e-6);

      /*
      const int numSteps = std::min(pow(10, timeExp), 1e2);
      for (int step = 0; step < numSteps; ++step) {
        double dt = context->dt * pow(10, timeExp) / numSteps;

        //printf("%f \n", dt);

        Vector3N newPos = rigidbody.position + rigidbody.velocity * dt + rigidbody.acceleration * (dt * dt) * 0.5;

        keplerians.each([&](auto &keplerian, auto &spatial){
          rigidbody.netForce += calcGravitationalForce(newPos, spatial.position, rigidbody.mass, keplerian.mass);
        });

        //printf("FORCE: %f %f %f\n", rigidbody.netForce.x, rigidbody.netForce.y, rigidbody.netForce.z);

        Vector3N newAcc = rigidbody.netForce / rigidbody.mass;
        Vector3N newVel = rigidbody.velocity + (rigidbody.acceleration + newAcc)*(dt*0.5);

        rigidbody.position = newPos;
        rigidbody.velocity = newVel;
        rigidbody.acceleration = newAcc;

        rigidbody.netForce = Vector3N{0, 0, 0};
        rigidbody.netTorque = Vector3N{0, 0, 0};
      }
        */
    });
  }
};
