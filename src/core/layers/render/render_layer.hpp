#pragma once

#include "core/resource_pool.hpp"
#include "core/layers/layer.hpp"
#include "core/math/matrix.hpp"
#include "core/renderer.hpp"
#include "core/layers/render/opengl/opengl_renderer.hpp"
#include "core/context.hpp"

#include <memory>
#include <vector>

struct RenderLayer : Layer {
  private:
    std::unique_ptr<Renderer> renderer;
    double t;
  public:
    void init() {
      renderer = std::make_unique<OpenGLRenderer>();
      
      renderer->resourcePool = context.resourcePool;
      renderer->sceneGraph = context.sceneGraph;
      renderer->init({800, 600, "engine"});
    }

    void draw() {
      //const std::vector<Camera> cameras = {cam};

      renderer->beginFrame();
      renderer->render();
      renderer->endFrame();

      if (renderer->shouldClose()) {
        shutdown();
        assert(false); // lol
        // call event to shutdown
      }
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
