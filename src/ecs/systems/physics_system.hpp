#pragma once

#include "core/math/constants.hpp"
#include "core/math/vector.hpp"
#include "ecs/system.hpp"

#include "ecs/components/rigidbody.hpp"
#include "ecs/components/spatial.hpp"

#include "core/math/utilities.hpp"

#include <math.h>

struct PhysicsSystem : System {
  void init() {
  }

  void update() {
    auto view = context->registry->view<Rigidbody, Spatial>();

    float s = context->runtime * 10;

    for (auto [entity, rigidbody, spatial] : view.each()) {
      spatial.rotation = eulerToQuaternion(s, s, 15);
    }

    /*
    for (auto [entity, rigidbody, spatial] : view.each()) {
      Vector3 acceleration;

      float width = (spatial.scale.x) * (spatial.scale.x) * 0.5;

      for (auto [e2, r2, s2] : view.each()) {
        Vector3 dr = spatial.position - s2.position;
        Number d = (dr.x * dr.x + dr.y * dr.y + dr.z * dr.z);
        
        if ((float)d > width) {
          acceleration = acceleration + (dr.normalized() * -1 * r2.mass) / d;
        }
        else if ((float)d > 0.01f) {
          rigidbody.mass += r2.mass;
          float s = std::cbrt(rigidbody.mass) * 0.2;
          spatial.scale = Vector3F(s, s, s);
          context->registry->destroy(e2);
          printf("New mass: %f\n", rigidbody.mass);
        }
      }

      rigidbody.velocity = rigidbody.velocity + acceleration * context->dt;
      spatial.position = spatial.position + rigidbody.velocity * context->dt;
    }
      */
  }
};
