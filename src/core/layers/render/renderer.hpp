#pragma once

#include "core/math/vector.hpp"
#include "core/command_buffer.hpp"
#include "core/resource_pool.hpp"

#pragma once
#include <vector>
#include <span>
#include <cstdint>
#include "core/math/vector.hpp"
#include "core/command_buffer.hpp"
#include "core/resource_pool.hpp"

#include "core/render_utils.hpp"
#include "ecs/components/camera.hpp"

struct WindowSettings {
  uint16_t width;
  uint16_t height;
  std::string title;
};

struct RenderGraph {

};

struct Renderer {
  ResourcePool* resourcePool;
  CommandBuffer* commandBuffer;

  virtual ~Renderer() = default;
  virtual void init(const WindowSettings settings, ResourcePool *pool) = 0;
  virtual void shutdown() = 0;
  virtual void beginFrame() = 0;
  virtual void endFrame() = 0;
  virtual void render() = 0;
  virtual Vector2I getSize() = 0;
  virtual bool shouldClose() = 0;
};
