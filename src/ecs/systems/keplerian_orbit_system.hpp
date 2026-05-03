#pragma once

#include "ecs/system.hpp"
#include "ecs/components/keplerian_orbiter.hpp"
#include "ecs/components/spatial.hpp"

struct KeplerianOrbitSystem : System {
  private:
  public:
    void init() {

    }

    void update() {
      auto view = context.registry->view<KeplerianOrbiter, Spatial>();
      view.each([this](auto &newtonianOrbiter, auto &spatial) {
        // do math here
      });
    }
};
