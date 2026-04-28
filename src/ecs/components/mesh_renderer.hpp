#pragma once

#include "core/resources/mesh.hpp"
#include "core/resources/material.hpp"
#include "core/handle.hpp"

// mesh renderer contains a reference of some sort to a mesh that the resource layer holds onto
// same deal with material
struct MeshRenderer {
  Handle<Mesh> mesh;
  Handle<Material> material;
};
