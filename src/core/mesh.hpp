#pragma once

#include "core/math/vector.hpp"
#include "vector"

struct Mesh {
  std::vector<Vector3> vertices;
  std::vector<Vector3> normals;
  std::vector<Vector2> uvs;
  std::vector<int> indices;
};
