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

  // ideally i want to be able to say material.uniforms["albedo_texture"] = some_image;
  // then this image would be loaded to the gpu and cached
  // you could also do material.uniforms["albedo_color"] = Vector4F{1, 1, 1, 1}; // or Color(1, 1, 1, 1);

  bool load() override {
    return true;
  }

  bool unload() override {
    return true;
  }
};
