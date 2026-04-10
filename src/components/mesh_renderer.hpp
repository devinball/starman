#pragma once

#include "core/mesh.hpp"
#include "core/material.hpp"

struct MeshRenderer {
  Mesh* mesh;
  Material* material;
};
