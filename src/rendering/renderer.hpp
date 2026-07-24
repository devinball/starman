#pragma once

#include "resources/mesh.hpp"
#include "resources/material.hpp"
#include "resources/image.hpp"
#include "ecs/components/spatial.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/point_light.hpp"
#include "core/resource_manager.hpp"
#include "core/input_buffer.hpp"

template <>
struct std::hash<std::pair<Handle<Mesh>, Handle<Material>>> {
  std::size_t operator()(const std::pair<Handle<Mesh>, Handle<Material>>& p) const {
    return std::hash<ResourceID>()(p.first.id) ^ (std::hash<ResourceID>()(p.second.id) << 1);
  }
};

struct Plane {
  Vector3F normal;
  float distance;
};

struct Frustrum {
  Plane top;
  Plane bottom;
  Plane right;
  Plane left;
  Plane far;
  Plane near;
};

Frustrum createFrustrum(Camera camera, Spatial spatial) {
  Frustrum frustrum;
  const float fovY = 10;
  const float aspect = 1;
  const float halfVSide = camera.far * std::tan(fovY * 0.5);
  const float halfHSide = halfHSide * aspect;
  //const Vector3F frontMultFar = zFar * cam.Front;

  return frustrum;
}

struct RenderTarget {
  int id;
  int width;
  int height;

  bool isScreen() const { return id == 0; }
};

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

  // with auto instancing!
  // things should be culled before calling draw!!
  // expects everything already relative to camera!!
  virtual void addMesh3D(Handle<Mesh>& mesh, Handle<Material>& material, Spatial& spatial) = 0;
  virtual void addImage2D(Handle<Image>& image, Spatial& spatial) = 0;
  virtual void addCamera3D(Camera& camera, Spatial& spatial) = 0;
  virtual void addPointLight(PointLight& pointLight, Spatial& spatial) = 0;
  virtual void addSpotLight(Vector3N position, Color color, float intensity, float range, float fov) = 0;
  virtual void addStarLight(Vector3N position, Color color, float range) = 0;

  // then RenderTarget with id = 0 draws to the screen.
};

// ideally i could just call something like
// camera.drawMesh(myMesh); // or
// renderer.drawMesh(camera, myMesh);
// camera.drawMesh(myMesh, myMaterial);
// renderer.drawMesh(camera, myMesh, myMaterial); // or
// renderer.drawMesh(myMesh, myMaterial);
// then i could easily create materials by loading them
// and just set texture easily
