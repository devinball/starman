#pragma once

#include <vector>
#include <inttypes.h>
#include <string>

#include "core/math/matrix.hpp"
#include "core/math/color.hpp"

struct RenderTarget {
  int id;
  int width;
  int height;

  bool isScreen() const { return id == 0; }
};

