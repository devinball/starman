#pragma once

#include "core/scene_graph.hpp"
#include "core/resource_manager.hpp"
#include "core/input_buffer.hpp"

// TODO: include fullscreen, vsync in OpenGLRenderer
struct WindowSettings {
  uint16_t width;
  uint16_t height;
  std::string title;
  bool fullscreen;
  bool vsync;
};

enum RenderMode { FILL, NORMAL, WIREFRAME, DEPTH, LIGHTING, MOTION_VECTORS, UV };

struct Renderer {
  std::shared_ptr<Context> context;

  virtual ~Renderer() = default;
  virtual void init(const WindowSettings settings) = 0;

  // make sure to clean everything up, destroy windows, deallocate
  // resources, etc
  virtual void shutdown() = 0;

  virtual void beginFrame() = 0;
  virtual void endFrame() = 0;

  virtual void render() = 0;

  virtual Vector2I getSize() = 0;
  virtual bool shouldClose() = 0;

  virtual void setRenderMode(RenderMode renderMode) = 0;

  virtual void submitCamera(bool doClear, int id, int priority, float fov, float near, float far, Color clearColor, Vector3 position, QuaternionF rotation) = 0;
  virtual void submitModel(Handle<Mesh> meshHandle, Handle<Material> materialHandle, Vector3 position, Vector3F scale, QuaternionF rotation) = 0;
  virtual void submitPointLight(float intensity, float range, Vector3 pos, Color color) = 0;
};
