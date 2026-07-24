#pragma once

#include "ecs/ecs.hpp"

#include <memory>

// forward declarations
struct SceneManager;
struct ResourceManager;
struct Renderer;
struct InputBuffer;

struct Context {
  bool running = true;
  float dt = 0.01; // default non-zero value to avoid errors dividing by dt;
  float frameTime = 0.01;
  float updateTime = 0.01;
  double simTime = 0;
  double runtime = 0;
  double physicsInterpolate = 0;
  std::shared_ptr<SceneManager> sceneManager;
  std::shared_ptr<ResourceManager> resourceManager;
  std::shared_ptr<Registry> registry;
  std::shared_ptr<Renderer> renderer;
  std::shared_ptr<InputBuffer> inputBuffer;

  Context(
    std::shared_ptr<SceneManager> sceneManager,
    std::shared_ptr<ResourceManager> resourceManager,
    std::shared_ptr<Registry> registry,
    std::shared_ptr<Renderer> renderer,
    std::shared_ptr<InputBuffer> inputBuffer
  ) {
    this->sceneManager = sceneManager;
    this->resourceManager = resourceManager;
    this->registry = registry;
    this->renderer = renderer;
    this->inputBuffer = inputBuffer;
  }
};
