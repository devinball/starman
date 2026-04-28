#pragma once

#include "core/resources/resource.hpp"
#include "core/handle.hpp"
#include "core/resources/shader.hpp"
#include "core/math/vector.hpp"
#include "core/resources/image.hpp"

#include <map>

struct Material : Resource {
  using Resource::Resource;

  Handle<Shader> shader;
  
  std::unordered_map<std::string, float> floats;
  std::unordered_map<std::string, Vector4F> vectors;
  std::unordered_map<std::string, Handle<Image>> textures;

  bool load() override {
    return true;
  }

  bool unload() override {
    return true;
  }
};
