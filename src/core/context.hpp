#pragma once

#include "core/scene_graph.hpp"
#include "core/resource_pool.hpp"
#include "ecs/ecs.hpp"

#include <memory>

struct Context {
  std::shared_ptr<float> dt;
  std::shared_ptr<SceneGraph> sceneGraph;
  std::shared_ptr<ResourcePool> resourcePool;
  std::shared_ptr<Registry> registry;
};
