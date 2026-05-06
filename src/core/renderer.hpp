#pragma once

#include "core/scene_graph.hpp"
#include "core/resource_pool.hpp"

struct WindowSettings {
  uint16_t width;
  uint16_t height;
  std::string title;
};

enum RenderMode { DEFAULT, NORMAL, WIREFRAME, DEPTH, LIGHTING, MOTION_VECTORS, UV };

struct Renderer {
  std::shared_ptr<ResourcePool> resourcePool;
  std::shared_ptr<SceneGraph> sceneGraph;

  virtual ~Renderer() = default;
  virtual void init(const WindowSettings settings) = 0;

  // make sure to clean everything up, destroy windows, deallocate
  // resources, etc
  virtual void shutdown() = 0;

  virtual void beginFrame() = 0;
  virtual void endFrame() = 0;

  // render should handle reading from the scene graph, ideally
  // a renderer should be able to render something even if the
  // provided material is empty, a default shader is a good here
  virtual void render() = 0;

  virtual Vector2I getSize() = 0;
  virtual bool shouldClose() = 0;

  virtual void setRenderMode(RenderMode renderMode) = 0;
};
