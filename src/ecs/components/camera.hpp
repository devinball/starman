#pragma once

#include "core/math/matrix.hpp"
#include "core/math/color.hpp"

// likely will also need render target
struct Camera {
  Matrix4x4F view;
  Matrix4x4F projection;
  Color clearColor = {0, 0, 0.2, 1};
  bool doClear = true;
};
