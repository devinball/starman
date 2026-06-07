#pragma once

#include "core/math/constants.hpp"
#include "core/math/vector.hpp"
#include "ecs/system.hpp"

#include "ecs/components/rigidbody.hpp"
#include "ecs/components/spatial.hpp"

struct PhysicsSystem : System {
  private:
    Number currentTime;

  public:
    void init() {
    }

    void update() {
      auto view = context->registry->view<Rigidbody, Spatial>();

      view.each([this](auto &rigidbody, auto &spatial){
        // TODO: collision check
        // TODO: might need to use proper time instead of coordinate time
        spatial.position = spatial.position + rigidbody.velocity * context->dt;

        //spatial.rotation = eulerToQuat(context->runtime, context->runtime, context->runtime);
      });

      /*
      view.each([this](auto &rigidbody, auto &spatial){
        // calculate trajectory, always at least enough for
        // this frame.

        int iterations = 100;

        float delta = 1/100;
        for (int step = 0; step < iterations; ++step) {
          Vector3 force = {0, 0, 0};

          Vector4 last = rigidbody.trajectory.end();
          // integrate forces
          rigidbody.trajectory.push_bacK(Vector4(
            last.x + force * delta,
            last.y + force * delta,
            last.z + force * delta,
            last.t + step * delta));
        }

        // then move object along trajectory, use either lerp
        // or polynomial fit

        auto p0 = rigidbody.trajectory[lastPositionIndex];
        auto p1 = rigidbody.trajectory[lastPositionIndex + 1];

        float t = 0.5; // should be set from weighted average between p0.w and p1.w

        float delta = p1.w - p0.w;

        // just keep track of the index we last reached
        // then lerp between n and n+1 based on n[time] and (n+1)[time]
        spatial.position = Vector3(lerp(p0.x, p1.x, t), lerp(p0.y, p1.y, t), lerp(p0.z, p1.z, t));

        rigidbody.velocity = Vector3(p0.x - p1.x, p0.y - p1.y, p0.z - p1.z) / (p1.w - p0.w);
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
