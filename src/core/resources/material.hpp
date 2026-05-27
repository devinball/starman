#pragma once

#include "core/resources/resource.hpp"
#include "core/handle.hpp"
#include "core/resources/shader.hpp"
#include "core/math/vector.hpp"
#include "core/resources/image.hpp"

#include "yaml-cpp/yaml.h"
#include <map>

struct Material : Resource {
  using Resource::Resource;

  std::string shaderId;//Handle<Shader> shader;
  
  std::unordered_map<std::string, float> floats;
  std::unordered_map<std::string, Vector4F> vectors;
  std::unordered_map<std::string, Handle<Image>> textures;

  // ideally i want to be able to say material.uniforms["albedo_texture"] = some_image;
  // then this image would be loaded to the gpu and cached
  // you could also do material.uniforms["albedo_color"] = Vector4F{1, 1, 1, 1}; // or Color(1, 1, 1, 1);

  void set(std::string key, float value) {
    floats[key] = value;
  }

  void set(std::string key, Vector4F value) {
    vectors[key] = value;
  }

  void set(std::string key, Handle<Image> value) {
    textures[key] = value;
  }

  bool load() override {
    YAML::Node config = YAML::LoadFile(getId().c_str());

    if (config["shader"]) {
      shaderId = config["shader"].as<std::string>();
    }

    if (config["properties"]) {
      for (const auto& property : config["properties"]) {
        for (const auto& kv : property) {
          std::string key = kv.first.as<std::string>();
          const YAML::Node& value = kv.second;

          if (value.IsSequence() && value.size() == 4) {
            set(key, Vector4F(
              value[0].as<float>(),
              value[1].as<float>(),
              value[2].as<float>(),
              value[3].as<float>()
            ));
          }
          else if (value.IsSequence() && value.size() == 3) {
            set(key, Vector4F(
              value[0].as<float>(),
              value[1].as<float>(),
              value[2].as<float>()
            ));
          }
          else if (value.IsSequence() && value.size() == 2) {
            set(key, Vector4F(
              value[0].as<float>(),
              value[1].as<float>()
            ));
          }
          else if (value.IsScalar()) {
            set(key, value.as<float>());
          }
          else {
            printf("Unrecognized property type");
          }
        }
      }
    }

    return true;
  }

  bool unload() override {
    floats.clear();
    vectors.clear();
    textures.clear();
    return true;
  }
};
