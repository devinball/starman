#pragma once

#include "math/vector.hpp"
#include "resources/resource.hpp"
#include "resources/shader.hpp"
#include "resources/texture.hpp"

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