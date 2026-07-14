#pragma once

#include "core/context.hpp"
#include "ecs/ecs.hpp"
#include "rendering/opengl_renderer.hpp"
#include "core/scene_manager.hpp"
#include "core/input_buffer.hpp"

#include <memory>
#include <chrono>

struct Application {
  std::shared_ptr<Context> context;

  float physicsFrequency = 60; // physics update rate in hertz

  void run() {
    context = std::make_shared<Context>(
      std::make_shared<SceneGraph>(),
      std::make_shared<SceneManager>(),
      std::make_shared<ResourceManager>(),
      std::make_shared<Registry>(),
      std::make_shared<OpenGLRenderer>(),
      std::make_shared<InputBuffer>()
    );

    context->dt = 1 / physicsFrequency;

    context->renderer->context = context;
    context->renderer->init({800, 600, "engine", false, false});

    context->sceneManager->context = context;
    context->sceneManager->init();

    float accumulator = 0;
    auto startTime = std::chrono::high_resolution_clock::now();
    auto previous = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed;

    while (context->running) {
      auto now = std::chrono::high_resolution_clock::now();
      elapsed = now - previous;
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
      elapsed = std::chrono::high_resolution_clock::now() - now;
      context->frameTime = elapsed.count();

      elapsed = std::chrono::high_resolution_clock::now() - startTime;
      context->runtime = elapsed.count();

      context->running = !context->renderer->shouldClose();

      //printf("Frametime: %f ms FPS: %i\n", context->frameTime * 1000, int(1 / context->frameTime));
    }

    context->renderer->shutdown();
  }
};
