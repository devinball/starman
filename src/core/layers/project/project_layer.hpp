#pragma once

#include "core/layers/layer.hpp"

// the loader layer is the very lowest
// it loads info from the project file
// for example the main scene

struct ProjectLayer : Layer {
  void update();
  void draw();
  void init();
  void event(Event event);
};