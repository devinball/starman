#pragma once

namespace Raylib {
  #include "raylib.h"
}

#include "core/math/vector.hpp"

struct RenderContext {
  private:
    Raylib::Camera3D camera;
  public:
    void init(int width, int height, char name[]) {
      Raylib::InitWindow(500, 500, "engine");
      Raylib::SetTargetFPS(0);

      camera.up = { 0, 1, 0 };
      camera.fovy = 45.0f;
      camera.projection = Raylib::CAMERA_PERSPECTIVE;
    }

    void update() {
      Raylib::BeginDrawing();
        Raylib::ClearBackground(Raylib::RAYWHITE);
        Raylib::BeginMode3D(camera);
          Raylib::DrawCube({0, 0, 0}, 2, 2, 2, Raylib::BLUE);
        Raylib::EndMode3D();
      Raylib::EndDrawing();
    }

    void setCameraPosition(Vector3 pos) {
      camera.position = { pos.x, pos.y, pos.z };
    }

    void setCameraTarget(Vector3 pos) {
      camera.target = { pos.x, pos.y, pos.z };
    }

    void shutdown() {
      Raylib::CloseWindow();
    }

    bool shouldClose() {
      return Raylib::WindowShouldClose();
    }
};
