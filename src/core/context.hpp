#pragma once

#include "ecs/ecs.hpp"

#include <memory>

// forward declarations
struct SceneGraph;
struct SceneManager;
struct ResourcePool;
struct Renderer;

struct Context {
  bool running = true;
  float dt = 0.01; // default non-zero value to avoid errors dividing by dt;
  float frameTime = 0.01;
  std::shared_ptr<SceneGraph> sceneGraph;
  std::shared_ptr<SceneManager> sceneManager;
  std::shared_ptr<ResourcePool> resourcePool;
  std::shared_ptr<Registry> registry;
  std::shared_ptr<Renderer> renderer;

  Context(
    std::shared_ptr<SceneGraph> sceneGraph,
    std::shared_ptr<SceneManager> sceneManager,
    std::shared_ptr<ResourcePool> resourcePool,
    std::shared_ptr<Registry> registry,
    std::shared_ptr<Renderer> renderer
  ) {
    this->sceneGraph = sceneGraph;
    this->sceneManager = sceneManager;
    this->resourcePool = resourcePool;
    this->registry = registry;
    this->renderer = renderer;
  }
};
