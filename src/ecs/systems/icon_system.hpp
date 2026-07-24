#pragma once

#include "ecs/system.hpp"
#include "core/context.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/icon.hpp"

#include "core/input_buffer.hpp"

#include <math.h>

#include "imgui.h"

// !! LLM WRITTEN !!
Vector2F worldToScreen(const Vector3N& worldPosition, const Camera& camera, const Spatial& spatial, int screenWidth, int screenHeight, bool* outInFrontOfCamera = nullptr) {
    // --- column-major 4x4 float math, no glm -------------------------------
    using Mat4 = std::array<float, 16>;
    auto at = [](Mat4& m, int row, int col) -> float& { return m[col * 4 + row]; };
 
    auto multiply = [&](const Mat4& a, const Mat4& b) {
        Mat4 r{};
        for (int col = 0; col < 4; ++col)
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) sum += a[k * 4 + row] * b[col * 4 + k];
                r[col * 4 + row] = sum;
            }
        return r;
    };
 
    // View matrix: inverse of the camera's rotation (= transpose, since it's
    // a pure rotation) with no translation, because we do the translation as
    // a high-precision vector subtraction below instead of baking it into
    // the matrix.
    Mat4 view{};
    {
        const float w = spatial.rotation.r, x = spatial.rotation.i,
                    y = spatial.rotation.j, z = spatial.rotation.k;
        // Rotation matrix from quaternion, pre-transposed (i.e. its inverse).
        at(view, 0, 0) = 1.0f - 2.0f * (y * y + z * z);
        at(view, 1, 0) = 2.0f * (x * y - w * z);
        at(view, 2, 0) = 2.0f * (x * z + w * y);
 
        at(view, 0, 1) = 2.0f * (x * y + w * z);
        at(view, 1, 1) = 1.0f - 2.0f * (x * x + z * z);
        at(view, 2, 1) = 2.0f * (y * z - w * x);
 
        at(view, 0, 2) = 2.0f * (x * z - w * y);
        at(view, 1, 2) = 2.0f * (y * z + w * x);
        at(view, 2, 2) = 1.0f - 2.0f * (x * x + y * y);
        at(view, 3, 3) = 1.0f;
    }
 
    // Perspective projection matrix (right-handed, NDC z in [-1, 1]).
    Mat4 proj{};
    {
        constexpr float kPi = 3.14159265358979323846f;
        const float aspect = float(screenWidth) / float(screenHeight);
        const float f = 1.0f / std::tan((camera.fov * kPi / 180.0f) * 0.5f);
        const float n = camera.near, fr = camera.far;
 
        at(proj, 0, 0) = f / aspect;
        at(proj, 1, 1) = f;
        at(proj, 2, 2) = (fr + n) / (n - fr);
        at(proj, 2, 3) = (2.0f * fr * n) / (n - fr);
        at(proj, 3, 2) = -1.0f;
    }
 
    const Mat4 viewProj = multiply(proj, view);
 
    // High-precision camera-relative subtraction, then drop to float.
    const Vector3D relative = {
        double(worldPosition.x - spatial.position.x),
        double(worldPosition.y - spatial.position.y),
        double(worldPosition.z - spatial.position.z)
    };
    const float rx = float(relative.x), ry = float(relative.y), rz = float(relative.z);
 
    // Transform by view-projection.
    const float clipX = viewProj[0] * rx + viewProj[4] * ry + viewProj[8] * rz + viewProj[12];
    const float clipY = viewProj[1] * rx + viewProj[5] * ry + viewProj[9] * rz + viewProj[13];
    const float clipW = viewProj[3] * rx + viewProj[7] * ry + viewProj[11] * rz + viewProj[15];
 
    if (outInFrontOfCamera) *outInFrontOfCamera = clipW > 1e-5f;
    if (clipW <= 1e-5f) return Vector2F{ 0.0f, 0.0f }; // behind the camera
 
    const float ndcX = clipX / clipW;
    const float ndcY = clipY / clipW;
 
    return Vector2F{
        (ndcX * 0.5f + 0.5f) * float(screenWidth),
        (1.0f - (ndcY * 0.5f + 0.5f)) * float(screenHeight) // flip Y: NDC is bottom-up, screen is top-down
    };
}

// icons should disappear when within like 10 radii

struct IconSystem : System {
  int count;

  void update() {
    auto icons = context->registry->view<Icon, Spatial>();

    icons.each([&](auto &icon, auto &spatial){
      icon.trajectory.push_back(spatial.position);
    });
  }

  void draw() {
    if (!context->inputBuffer->tab) { return; }

    auto cameras = context->registry->view<Camera, Spatial>();
    auto icons = context->registry->view<Icon, Spatial>();

    cameras.each([&](auto &camera, auto &camSpatial){
      icons.each([&](auto &icon, auto &spatial){
        Vector2I size = context->renderer->getSize();

        /*
        std::vector<ImVec2> points;
        for (auto pos3 : icon.trajectory) {
          Vector2F pos = worldToScreen(pos3, camera, camSpatial, size.x, size.y);
          if (pos != Vector2F{0, 0}) {
            points.push_back({pos.x, pos.y});
          }
        }

        ImGui::GetBackgroundDrawList()->AddPolyline(points.data(), (int)points.size(), IM_COL32(
          int(icon.color.r * 255),
          int(icon.color.g * 255),
          int(icon.color.b * 255),
          int(icon.color.a * 255)
        ), 2.f);
        */

        Vector2F pos = worldToScreen(spatial.position, camera, camSpatial, size.x, size.y);
        if (pos != Vector2F{0, 0}) {
          ImGui::GetBackgroundDrawList()->AddCircle(
          {pos.x, pos.y}, 5,
          IM_COL32(
            int(icon.color.r * 255),
            int(icon.color.g * 255),
            int(icon.color.b * 255),
            int(icon.color.a * 255)
          ), 0, 0);
        }
      });
    });
  }
};
