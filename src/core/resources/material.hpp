#pragma once

#include "core/resources/resource.hpp"
#include "core/math/vector.hpp"
#include "core/filesystem/file.hpp"

#include "rfl.hpp"
#include "rfl/yaml.hpp"

struct Material : Resource {
  using Resource::Resource;

  std::string shaderPath;
  
  std::unordered_map<std::string, float> floats;
  std::unordered_map<std::string, Vector4F> vectors;
  std::unordered_map<std::string, std::string> textures;

  void set(std::string key, float value) {
    floats[key] = value;
  }

  void set(std::string key, Vector4F value) {
    vectors[key] = value;
  }

  void set(std::string key, std::string value) {
    textures[key] = value;
  }

  struct Config {
    std::string shader;
    std::string filterMode;
    std::unordered_map<std::string, rfl::Variant<float, std::string, std::array<float, 4>>> properties;
  };

  bool load(const std::string &path) {
    auto result = rfl::yaml::read<Config>(readFile(path)).value();

    shaderPath = result.shader;

    for (const auto& prop : result.properties) {
      const auto& [key, value] = prop;

      const auto visitor = [&](const auto& v) {
        using Type = std::decay_t<decltype(v)>;

        if constexpr (std::is_same<Type, float>()) {
          set(key, v);
        }
        else if constexpr (std::is_same<Type, std::string>()) {
          set(key, v);
        }
        else if constexpr (std::is_same<Type, std::array<float, 4>>()) {
          set(key, Vector4F(v[0], v[1], v[2], v[3]));
        }
        else {
          printf("Could not load type in material: %s\n", path.c_str());
        }
      };

      value.visit(visitor);
    }

    return true;
  }

  bool unload() {
    floats.clear();
    vectors.clear();
    textures.clear();
    return true;
  }
};
