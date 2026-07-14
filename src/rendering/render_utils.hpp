#pragma once

#include <vector>
#include <inttypes.h>
#include <string>

struct RenderTarget {
  int id;
  int width;
  int height;

  bool isScreen() const { return id == 0; }
};

