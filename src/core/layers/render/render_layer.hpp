#pragma once

#include "core/command_buffer.hpp"
#include "core/resource_pool.hpp"
#include "core/layers/layer.hpp"
#include "core/math/matrix.hpp"
#include "core/layers/render/renderer.hpp"
#include "core/layers/render/opengl/opengl.hpp"
#include "core/context.hpp"

#include <memory>
#include <vector>

// --- LLM WRITTEN ---
// also i need to move this function somewhere better
Matrix4x4F perspective_projection(
    float fovy_radians,
    float aspect_ratio,
    float near_plane,
    float far_plane
) {

    Matrix4x4F result;

    const float tan_half_fovy = std::tan(fovy_radians / 2);

    result(0, 0) = 1 / (aspect_ratio * tan_half_fovy);
    result(1, 1) = 1 / tan_half_fovy;
    result(2, 2) = - (far_plane + near_plane) / (far_plane - near_plane);
    result(2, 3) = - 1;
    result(3, 2) = - (2 * far_plane * near_plane) / (far_plane - near_plane);

    // Set remaining elements to 0
    for (uint8_t i = 0; i < 4; ++i) {
        for (uint8_t j = 0; j < 4; ++j) {
            if (i != j && !(i == 2 && j == 3) && !(i == 3 && j == 2)) {
                result(i, j) = 0;
            }
        }
    }

    return result;
}

struct RenderLayer : Layer {
  private:
    std::unique_ptr<Renderer> renderer;
    double t;
  public:
    void init() {
      renderer = std::make_unique<OpenGLRenderer>();
      
      renderer->init(WindowConfig{}, context.resourcePool);
    }

    void draw() {
      t += 0.01;
      Matrix4x4F m1({
          1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 1, 0,
          0, -2, -t, 1
      });

      Matrix4x4F projection = perspective_projection(
          3.1515926f / 3.0f, // 60 degrees in radians
          1280.0f / 720.0f, // aspect ratio
          0.1f, // near plane
          1000.0f // far plane
      );

      Camera cam(
        m1,
        projection,
        RenderTarget{}
      );

      const std::vector<Camera> cameras = {cam};

      renderer->beginFrame();
      renderer->render(context.commandBuffer->getCommands(), cameras);
      renderer->endFrame();
      context.commandBuffer->clear();

      //if (renderer->shouldClose()) {
        // call event to shutdown
      //}
    }

    void update() {
      // t += 1.0 / 60.0;
      // float speed = 5;
      // renderer->setCameraPosition({-5, 50, -5});
      // renderer->setCameraTarget({0, 0, 0});
      // renderer->setCameraPosition({sin(speed * t) * 5, sin(speed * t * 0.125f) * 5, cos(speed * t) * 5});
      // renderer->setCameraTarget({0, 0, 0});
    }

    void shutdown() {
      renderer->shutdown();
    }
};
