#pragma once

#include "ecs/system.hpp"
#include "ecs/components/newtonian_orbiter.hpp"
#include "ecs/components/celestial_body.hpp"
#include "ecs/components/spatial.hpp"

struct NewtonianOrbitSystem : System {
  private:
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
