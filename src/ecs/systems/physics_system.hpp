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
    return Vector3N(
      x,
      z,
      y
    );
  }

  Vector3N calcGravitationalForce(Vector3N a, Vector3N b, float mass1, float mass2) {
    Vector3N dr = b - a;
    Number d2 = dr.squareMagnitude();
    if (d2 > 1) {
      Number magnitude = 6.6743015e-11 * mass1 * mass2 / d2;
      return dr.normalized() * magnitude;
    }
    return {0, 0, 0};
  }

  void init() {
    auto spatials = context->registry->view<Rigidbody, Spatial>();

    spatials.each([&](auto &rigidbody, auto &spatial){
      rigidbody.position = spatial.position;
      rigidbody.rotation = spatial.rotation;

      rigidbody.velocity = {0, 0, 10000};
    });
  }

  // this allows me to interpolate, so the physics engine can
  // have it's own position kept precisely, and the renderer
  // can see interpolated motion.

  // we want to be able to set position via the spatial
  // rigidbody.position is physics internal only
  // it should probably detect if spatial.position has been
  // externaly changed, and update rigidbody.position to track
  void draw() {
    ImGui::SliderInt("Time Exp", &timeExp, 0, 10/*3600 * 24 * 365*/);

    auto spatials = context->registry->view<Rigidbody, Spatial>();

    spatials.each([&](auto &rigidbody, auto &spatial){
      spatial.position = rigidbody.position;
      spatial.rotation = rigidbody.rotation;
    });

    auto keplerians = context->registry->view<KeplerianOrbiter, Spatial>();  

    keplerians.each([this](auto &keplerian, auto &spatial) {
      double M = 2 * std::numbers::pi * t / keplerian.orbitalPeriod;
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

      //printf("%f %f %f %f %f %f %f\n", float(keplerian.orbitalPeriod), float(t), float(E), float(M), float(pos.x), float(pos.y),  float(pos.z));

      if (context->registry->valid(keplerian.parent)) {
        keplerian.barycenter = context->registry->get<Spatial>(keplerian.parent).position;
      }

      spatial.position = pos + keplerian.barycenter;
    });
  }

  void update() {
    t += context->dt * pow(10, timeExp);

    auto rigidbodies = context->registry->view<Rigidbody>();
    auto keplerians = context->registry->view<KeplerianOrbiter, Spatial>();

    rigidbodies.each([&](auto &rigidbody){
      /*
      // implicit euler, need to swap with velocity-verlet, or some other better energy-preserving integrator
      rigidbody.angularMomentum += rigidbody.netTorque * context->dt;
      rigidbody.angularVelocity = rigidbody.angularMomentum * (1 / rigidbody.momentInertia); // TODO : multiply by inverse momentInteria matrix instead
      rigidbody.rotation += rigidbody.rotation * rigidbody.angularVelocity * 0.5f * context->dt;
      rigidbody.rotation = rigidbody.rotation.normalized();

      rigidbody.linearMomentum += rigidbody.netForce * context->dt;
      rigidbody.linearVelocity = (rigidbody.linearMomentum * (1 / rigidbody.mass));
      rigidbody.position += toVector3N(rigidbody.linearVelocity) * context->dt;
      */

      // REALLY REALLY weird sudden forces

      // ok, so sometimes a very large negative number is passed to Number sqrt()
      // so somewhere, somehow x*x + y*y + z*z < 0
      // this doesn't make any sense, should only be possible if there are complex numbers
      // so somewhere, multiplication is being done wrong
      // !! what if it's integer overflow!!
      // a * b > max(int), so it wraps into being very small
      // ok then, but why then would (p1 - p2).magnitude()
      // suddenly be absurdly large, distance should vary pretty continously
      // ok, i need to test the number class rigorously
      // then patch any issues

      //double energy = 0.5 * rigidbody.mass * rigidbody.velocity.magnitude() + 6743015e-11 * rigidbody.mass * 1989100e24 / rigidbody.position.magnitude();
      //printf("Total energy: %f EJ | Speed: %f KM/S | Distance: %f MM \n", energy * 1e-18, rigidbody.velocity.magnitude() * 1e-3, rigidbody.position.magnitude() * 1e-6);

      /*
      Number x = rigidbody.position.x;
      Number y = rigidbody.position.y;
      Number z = rigidbody.position.z;
      Number x2 = x * x;
      Number y2 = y * y;
      Number z2 = z * z;
      Number r = x + y + z;
      Number r2 = x2 + y2 + z2;

      printf("%f %f %f %f %f %f %f %f\n",
        double(x),
        double(y),
        double(z),
        double(x2),
        double(y2),
        double(z2),
        double(r),
        double(r2)
      );
      */

      // WHEN INTERACTIONS ARE MORE COMPLEX MORE STEPS ARE NEEDED, THIS IS LIKELY TO OCCUR WHEN THE BODY IS CLOSER
      // TO IT'S PERIAPSIS, OR WHEN IT IS CLOSE TO MANY BODIES
      // janky substepping, maybe this is the solution to variable dt?
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
    });
  }
};
