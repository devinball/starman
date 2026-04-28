#pragma once

#include "core/math/vector.hpp"
#include "core/resources/resource.hpp"
#include "core/handle.hpp"
#include "core/resources/shader.hpp"
#include "core/resources/texture.hpp"

#include <vector>

struct Model {
  std::vector<Vector3F> vertices;
  std::vector<Vector3F> normals;
  std::vector<Vector2F> uvs;
  std::vector<int> indices;
  Handle<Shader> shader;
  std::unordered_map<std::string, float> floats;
  std::unordered_map<std::string, Vector4F> vectors;
  std::unordered_map<std::string, Handle<Texture>> textures;
};