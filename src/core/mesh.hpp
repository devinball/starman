#pragma once

#include "core/math/vector.hpp"
#include "vector"

struct Mesh {
  std::vector<Vector3> verticies;
  std::vector<Vector3> normals;
  std::vector<Vector2> uvs;
  std::vector<int> indicies;
};

