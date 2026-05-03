#pragma once

#include "core/math/vector.hpp"
#include "core/render_utils.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/point_light.hpp"
#include "core/handle.hpp"
#include "core/resources/mesh.hpp"
#include "core/resources/material.hpp"

#include <vector>
#include <span>
#include <cstdint>
#include <map>
#include <functional>

template <>
struct std::hash<std::pair<Handle<Mesh>, Handle<Material>>> {
  std::size_t operator()(const std::pair<Handle<Mesh>, Handle<Material>>& p) const {
    return std::hash<string>()(p.first.getId()) ^ (std::hash<string>()(p.second.getId()) << 1);
  }
};

struct SceneGraph {
  // goal is to have everything that is the same
  // material and mesh be represented in the same
  // object.
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

  std::unordered_map<std::pair<Handle<Mesh>, Handle<Material>>, std::vector<Matrix4x4F>> models;
  std::vector<PointLight> pointLights;
  //std::vector<GuiElement> guiElements;
  //std::vector<PostProcess> postProcesses;

  // Mesh has many Materials has many Matrix
  // std::unordered_map<Handle<Mesh>, std::unordered_map<Handle<Material>, std::vector<Matrix4x4F>> models;

  void submitModel(Handle<Mesh> meshHandle, Handle<Material> materialHandle, Matrix4x4F transform) {
    models[std::pair(meshHandle, materialHandle)].push_back(transform);
  }

  void eachModel(Matrix4x4F frustrum, std::function<void(Handle<Mesh>, Handle<Material>, std::vector<Matrix4x4F>&)> f) {
    for (auto& [key, transforms] : models) {
      auto& [meshHandle, materialHandle] = key;
      f(meshHandle, materialHandle, transforms);
    }
  }

  void clear() {
    models.clear();
  }
};