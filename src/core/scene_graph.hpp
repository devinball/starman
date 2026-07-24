#pragma once

#include "math/vector.hpp"
#include "math/quaternion.hpp"
#include "math/utilities.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/point_light.hpp"
#include "resources/mesh.hpp"
#include "resources/material.hpp"

#include <vector>
#include <span>
#include <cstdint>
#include <map>
#include <functional>

struct CameraData {
  bool doClear = true;
  int id = 0;
  int priority = 0;
  float fov = 90.f;
  float near = 1.f;
  float far = 10000.f;
  Color clearColor = {0, 0, 0.2, 1};
  Vector3N position;
  QuaternionF rotation;
};

struct ModelData {
  Vector3N position;
  Vector3F scale;
  QuaternionF rotation;
};

struct PointLightData {
  float intensity; // w/m^2
  float range; // m
  Vector3N position;
  Color color;
};

/*
Frustrum createFrustrum(const CameraData& cam, float aspect) {
  Frustrum frustrum;
  
  const float halfVSide = cam.far * tanf(cam.fov * 0.5f * (M_PI / 180.f));
  const float halfHSide * aspect;

  const Vector3F forward = cam.rotation * Vector3F(0.f, 0.f, -1.f);
  const Vector3F right = cam.rotation * Vector3F(1.f, 0.f, 0.f);
  const Vector3F up = cam.rotation * Vector3F(0.f, 1.f, 0.f);

  const Vector3F near = cam.position + forward * cam.near;
  const Vector3F far = cam.position + forward * cam.far;

  const Vector3F pos({0,0,0}); // remember to shift everything by -cam.position

  frustrum.near = { forward, forward * near };
  frustrum.far = { -forward, -forward * far };
  
  frustrum.right = {
    (up.cross(far + right * halfHSide - pos)).normalize(),
    frustrum.right.normal * pos
  };
  frustrum.left = {
    (-up.cross(far + right * halfHSide - pos)).normalize(),
    frustrum.left.normal * pos
  };

  frustrum.top = {
    (right.cross(far + up * halfVSide - pos)).normalize(),
    frustrum.top.normal * pos
  };
  frustrum.bottom = {
    (-right.cross(far + up * halfVSide - pos)).normalize(),
    frustrum.bottom.normal * pos
  };

  return frustrum;
}
*/

struct SceneGraph {
  // goal is to have everything that is the same
  // material and mesh be represented in the same object.
  // so i need a multimap of a pair<Mesh, Material> -> many <Matrix4x4F>
  // ultimatly a single instance can have a ton of different locations,
  // which lends itself nicely to instanced drawing

  // the renderer then reads this map and traverses it,
  // for each mesh-material pair it draws the transforms
  // instanced if there are more then one, otherwise
  // it just draws normally.

  // then maybe functions to do frustrum culling,
  // then the renderer just requests the graph
  // with something like SceneGraph.eachModel(frustrum, [](count, material, mesh) {
  // 
  // });
  // each model loops over every model, HOWEVER, if a model is outside the frustrum (Matrix4x4F), then
  // the callback is not called for that element. this should provide a pretty simple way
  // to do culling.

  // there would be a SceneGraph.submitModel that would deal with adding a model to the multimap

  std::unordered_map<std::pair<Handle<Mesh>, Handle<Material>>, std::vector<ModelData>> models;
  std::unordered_map<int, CameraData> cameras;
  std::vector<PointLightData> pointLights;
  //std::vector<GuiElement> guiElements;
  //std::vector<PostProcess> postProcesses;

  // Mesh has many Materials has many Matrix
  //std::unordered_map<Handle<Mesh>, std::unordered_map<Handle<Material>, std::vector<Matrix4x4F>> models;

  void submitCamera(bool doClear, int id, int priority, float fov, float near, float far, Color clearColor, Vector3N position, QuaternionF rotation) {
    cameras[id] = CameraData{
      doClear,
      id,
      priority,
      fov,
      near,
      far,
      clearColor,
      position,
      rotation
    };
  }

  void eachCamera(std::function<void(CameraData& cameraData)> f) {
    for (auto& [key, cameraData] : cameras) {
      f(cameraData);
    }
  }

  void submitModel(Handle<Mesh> meshHandle, Handle<Material> materialHandle, Vector3N position, Vector3F scale, QuaternionF rotation) {
    //models[meshHandle][materialHandle].push_back(transform);
    models[std::pair(meshHandle, materialHandle)].push_back(ModelData{position, scale, rotation});
  }

  void eachModel(Frustrum frustrum, std::function<void(Handle<Mesh>, Handle<Material>, std::vector<ModelData>&)> f) {
    for (auto& [key, modelData] : models) {
      auto& [meshHandle, materialHandle] = key;
      f(meshHandle, materialHandle, modelData);
    }
  }

  void submitPointLight(float intensity, float range, Vector3N pos, Color color) {
    pointLights.push_back(PointLightData{intensity, range, pos, color});
  }

  int numModels() {
    return models.size();
  }

  void clear() {
    models.clear();
    cameras.clear();
    pointLights.clear();
  }
};
