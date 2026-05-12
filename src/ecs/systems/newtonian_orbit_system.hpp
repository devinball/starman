#pragma once

#include "ecs/system.hpp"
#include "ecs/components/newtonian_orbiter.hpp"
#include "ecs/components/celestial_body.hpp"
#include "ecs/components/spatial.hpp"

struct NewtonianOrbitSystem : System {
  private:
      Vector3 calculateGravitationalForce(float m2, Vector3 p1, Vector3 p2) {
        Vector3 delta = p1 - p2;
        Number r = delta.magnitude();
        Vector3 d = delta.normalized();
        return d * Constants::G * m2 / (r * r);
      }
  public:
    void init() {
    }

    void update() {
      auto orbiterView = context.registry->view<NewtonianOrbiter, Spatial>();
      orbiterView.each([this](auto &newtonianOrbiter, auto &spatial) {
        bodyView = context.registry->view<CelestialBody, Spatial>();
        // n-body
      });
    }
};
