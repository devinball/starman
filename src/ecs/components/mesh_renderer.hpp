#pragma once

#include "resources/mesh.hpp"
#include "resources/material.hpp"
#include "resources/resource.hpp"

// mesh renderer contains a reference of some sort to a mesh that the resource layer holds onto
// same deal with material
struct MeshRenderer {
  Handle<Mesh> mesh;
  Handle<Material> material;
};
