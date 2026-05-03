#pragma once

#include "core/math/constants.hpp"
#include "core/math/vector.hpp"
#include "ecs/system.hpp"

#include "components/rigidbody.hpp"
#include "components/spatial.hpp"

struct PhysicsSystem : System {
  private:
    Vector3 calculateGravitationalForce(float m2, Vector3 p1, Vector3 p2) {
      Vector3 delta = p1 - p2;
      Number r = delta.magnitude();
      Vector3 d = delta.normalized();
      return d * Constants::G * m2 / (r * r);
    }
  public:
    void init(Context& context) {
    }

    void update() {
      auto view = context.registry.view<Rigidbody, Spatial>();

      view.each([](auto &rigidbody, auto &spatial){
        // TODO: collision check
        spatial.position += spatial.velocity * context.dt;
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
