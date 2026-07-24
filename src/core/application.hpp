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

    float accumulator = 0.f;
    auto startTime = std::chrono::high_resolution_clock::now();
    auto previous = startTime;

    while (context->running) {
      auto now = std::chrono::high_resolution_clock::now();

      std::chrono::duration<float> elapsedSeconds = now - previous;
      float frameTime = elapsedSeconds.count();
      previous = now;

      if (frameTime <= 0.f) { frameTime = 0.00001f; }

      context->frameTime = frameTime;

      //printf("Frametime: %f ms FPS: %i\n", context->frameTime, int(1 / context->frameTime));

      accumulator += frameTime;

      while (accumulator >= context->dt) {
        context->sceneManager->update();
        context->simTime += context->dt;
        accumulator -= context->dt;
      }

      context->updateTime = context->dt;

      context->physicsInterpolate = accumulator * physicsFrequency; // same as dividing by dt
      
      context->renderer->beginFrame();
      context->sceneManager->draw();
      context->renderer->render();
      context->renderer->endFrame();

      context->runtime = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - startTime).count();

      context->running = !context->renderer->shouldClose();
    }

    context->renderer->shutdown();
  }
};
