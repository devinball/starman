#pragma once

#include "math/matrix.hpp"
#include "math/color.hpp"

// likely will also need render target
struct Camera {
  // might not need this, given that a camera requires a spatial
  // i can construct the view matrix from the spatial data
  // Matrix4x4F view;
  bool doClear = true;
  int id = 0; // should be random uuid
  int priority = 0;
  float fov = 90.f;
  float near = 0.1f;
  float far = 10000.f;
  //std::shared_ptr<RenderTarget> renderTarget;
  Color clearColor = {0, 0, 0.2, 1};
};
