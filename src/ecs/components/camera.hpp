#pragma once

#include "core/math/matrix.hpp"
#include "core/math/color.hpp"
#include "core/render_utils.hpp"

// likely will also need render target
struct Camera {
  // might not need this, given that a camera requires a spatial
  // i can construct the view matrix from the spatial data
  // Matrix4x4F view;
  int id = 0; // should be random uuid
  float fov;
  //std::shared_ptr<RenderTarget> renderTarget;
  Color clearColor = {0, 0, 0.2, 1};
  bool doClear = true;
  int priority = 0;
};
