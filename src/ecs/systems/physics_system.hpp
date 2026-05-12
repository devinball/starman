#pragma once

#include "core/math/constants.hpp"
#include "core/math/vector.hpp"
#include "ecs/system.hpp"

#include "ecs/components/rigidbody.hpp"
#include "ecs/components/spatial.hpp"

struct PhysicsSystem : System {
  public:
    void init() {
    }

    void update() {
      auto view = context->registry->view<Rigidbody, Spatial>();

      view.each([this](auto &rigidbody, auto &spatial){
        // TODO: collision check
        // TODO: might need to use proper time instead of coordinate time
        spatial.position = spatial.position + rigidbody.velocity * context->dt;

        // TODO: in the future when integrating forces I can use this equation:
        // γ^3 * m * a, where γ is the lorenz factor, and m is rest mass
        // or: a=\frac{F}{m}\left(1-\frac{v^{2}}{c^{2}}\right)^{\frac{3}{2}}

        // temporary make everything spin
        spatial.rotation = eulerToQuat(context->runTime, context->runTime, context->runTime);
      });

      /*
      view.each([](auto &rigidbody, auto &spatial) {
        Vector3 force = {0, 0, 0};
        view.each([](auto &rigidbody2, auto &spatial2) {
          force += calculateGravitationalForce(rigidbody2.mass, spatial.position, spatial2.position);
        });
        // f = G m_1 * m_2 / r^2
        // a = f/m
        // a = G * m_2 / r^2
        rigidbody.velocity += force / dt;
      });
      */
    }
};
