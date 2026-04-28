#pragma once

#include "core/math/matrix.hpp"
#include "core/math/color.hpp"

#include "core/render_utils.hpp"

// --- LLM written ---
struct Camera {
  Matrix4x4F view;
  Matrix4x4F projection;
  RenderTarget target; // invalid = screen
  Color clearColor = {0.1f, 0.1f, 0.1f, 1.f};
  bool doClear = true;
  uint8_t layerMask = 0xFF; // render all layers
  int priority = 0;         // sort order
};
