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
