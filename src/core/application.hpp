#pragma once

#include "core/context.hpp"
#include "ecs/ecs.hpp"
#include "core/layers/layer.hpp"
#include "core/rendering/opengl/opengl_renderer.hpp"
#include "core/scene_manager.hpp"

#include <memory>
#include <chrono>

struct Application {
  std::shared_ptr<Context> context;

  float physicsFrequency = 250; // physics update rate in hertz

  // application holds onto layers
  // we can add and remove layers from the stack
  // application will by default add a bunch of layers
  // to the stack-> maybe in future this can be disabled
  // via build flags.

  void run() {
    // some of these, especially the renderer may need to
    // be moved to their own initialization functions, in
    // the case of the renderer for example this would find
    // the correct backend to use.
    context = std::make_shared<Context>(
      std::make_shared<SceneGraph>(),
      std::make_shared<SceneManager>(),
      std::make_shared<ResourcePool>(),
      std::make_shared<Registry>(),
      std::make_shared<OpenGLRenderer>()
    );

    context->dt = 1 / physicsFrequency; // well it should be anyways

    context->renderer->resourcePool = context->resourcePool;
    context->renderer->sceneGraph = context->sceneGraph;
    context->renderer->init({800, 600, "engine"});

    context->sceneManager->context = context; // this feels wrong...
    context->sceneManager->init();

    float accumulator = 0;
    auto previous = std::chrono::high_resolution_clock::now();

    while (context->running) {
      auto now = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed = now - previous;
      previous = now;
      accumulator += elapsed.count();

      while (accumulator >= context->dt) {
        context->sceneManager->update();
        accumulator -= context->dt;
      }
      
      context->sceneManager->draw();

      context->renderer->beginFrame();
      context->renderer->render();
      context->renderer->endFrame();

      // reuse now from physics loop calculation for begin frame time
      std::chrono::duration<double> timer = std::chrono::high_resolution_clock::now() - now;
      context->frameTime = timer.count();

      context->running = !context->renderer->shouldClose();
    }

    context->renderer->shutdown();
  }
};
