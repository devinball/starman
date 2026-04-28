#pragma once

#include "core/command_buffer.hpp"
#include "core/resource_pool.hpp"
#include "ecs/ecs.hpp"

struct Context {
  CommandBuffer* commandBuffer;
  ResourcePool* resourcePool;
  Registry* registry;
};
