#pragma once

#include "ecs/system.hpp"
#include "core/command_buffer.hpp"
#include "core/context.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/mesh_renderer.hpp"

struct RenderSystem : System {
  private:
    void localizeMesh(); // convert from Number space to local float

    float t;
  public:
    void init() {
    }

    void update() {
    }

    Matrix4x4F transformMatrix(Vector3F position, Vector3F scale, QuaternionF rotation) {
      Matrix4x4F translation({
          1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          position.x, position.y, position.z, 1
      });

      // Scaling matrix
      Matrix4x4F scaling({
          scale.x, 0, 0, 0,
          0, scale.y, 0, 0,
          0, 0, scale.z, 0,
          0, 0, 0, 1
      });

      // Rotation matrix from quaternion
      float xx = rotation.r * rotation.r;
      float yy = rotation.i * rotation.i;
      float zz = rotation.k * rotation.k;
      float xy = rotation.r * rotation.i;
      float xz = rotation.r * rotation.k;
      float yz = rotation.i * rotation.k;
      float wx = rotation.j * rotation.r;
      float wy = rotation.j * rotation.i;
      float wz = rotation.j * rotation.k;

      Matrix4x4F rotationMatrix({
        1 - 2 * (yy + zz), 2 * (xy - wz), 2 * (xz + wy), 0,
        2 * (xy + wz), 1 - 2 * (xx + zz), 2 * (yz - wx), 0,
        2 * (xz - wy), 2 * (yz + wx), 1 - 2 * (xx + yy), 0,
        0, 0, 0, 1
      });

      Matrix4x4F transform = scaling * rotationMatrix * translation;
      return transform;
    }

    void draw() {
      t += 0.01;
      auto view = context.registry->view<MeshRenderer, Spatial>();
      view.each([this](auto &mesh_renderer, auto &spatial){
        DrawCommand cmd = {
          0,
          1,
          mesh_renderer.mesh,
          Handle<Material>(),
          transformMatrix(
            Vector3F{(float)spatial.position.x, (float)spatial.position.y, (float)spatial.position.z},
            Vector3F{(float)spatial.scale.x, (float)spatial.scale.y, (float)spatial.scale.z},
            euler_angles_to_quat(t, t, t) //spatial.rotation
          )
        };

        context.commandBuffer->submit(cmd);
      });
    }
};
