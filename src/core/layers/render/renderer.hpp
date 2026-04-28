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

struct Renderer
{
  virtual ~Renderer() = default;

  virtual void init(const WindowConfig &cfg, ResourcePool *pool) = 0;
  virtual void shutdown() = 0;

  virtual void beginFrame() = 0;
  virtual void endFrame() = 0;

  virtual RenderTarget createRenderTarget(uint32_t width, uint32_t height) = 0;
  virtual void destroyRenderTarget(RenderTarget rt) = 0;
  virtual void render(const std::vector<DrawCommand> &commands, const std::vector<Camera> &cameras, const EvictionList &evictions = {}) = 0;
  virtual bool shouldClose() const = 0;
  virtual void setTitle(const char *t) = 0;
  virtual const char *backendName() const = 0;
};

/*
struct Renderer
{
public:
  virtual void init(int width, int height, char name[]) {}
  virtual void beginFrame() { printf("beginFrame\n"); }
  virtual void endFrame() { printf("endFrame\n"); }
  virtual void executeCommands(std::vector<DrawCommand> drawCommands) {}
  virtual void setCameraPosition(Vector3 pos) {}
  virtual void setCameraTarget(Vector3 pos) {}
  virtual void shutdown() {}
  virtual bool shouldClose() { return true; }
};

*/