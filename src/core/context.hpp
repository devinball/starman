#pragma once

#include "core/scene_graph.hpp"
#include "core/resource_pool.hpp"
#include "ecs/ecs.hpp"

#include <memory>

struct Context {
  Context(
    std::shared_ptr<SceneGraph> sceneGraph,
    std::shared_ptr<ResourcePool> resourcePool,
    std::shared_ptr<Registry> registry
  ) {

      this->sceneGraph = sceneGraph;
      this->resourcePool = resourcePool;
      this->registry = registry;
  }

  bool running = true;
  float dt = 0.01; // default non-zero value to avoid errors dividing by dt;
  float frameTime = 0.01;
  std::shared_ptr<SceneGraph> sceneGraph;
  std::shared_ptr<ResourcePool> resourcePool;
  std::shared_ptr<Registry> registry;
};
