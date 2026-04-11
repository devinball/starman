#pragma once

#include "raylib.h"

struct RaylibContext {};

RaylibContext createRaylibContext() {
  InitWindow(500, 500, "engine");
  SetTargetFPS(0);
  
  return {};
}

void destroyRaylibContext() {
  CloseWindow();
}

bool shouldClose() {
  return WindowShouldClose();
}
