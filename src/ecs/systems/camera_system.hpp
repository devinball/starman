#pragma once

#include "ecs/system.hpp"
#include "core/context.hpp"

#include "ecs/components/tag.hpp"
#include "ecs/components/spatial.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/camera_controller.hpp"

#include "core/input_buffer.hpp"

#include <math.h>

#include "imgui.h"

struct CameraSystem : System {
  private:
    void localizeMesh(); // convert from Number space to local float

    float t;

    Spatial defaultFrame = {Vector3N{0, 0, 0}, Vector3F{1, 1, 1}, QuaternionF()};
    Vector3N camPos;
    Vector3N frame;
    Entity frameEntity;
  public:
    void draw() {
      auto frames = context->registry->view<Tag, Spatial>();

      for (auto [entity, tag, spatial] : frames.each()) {
        if (ImGui::Button(tag.name.c_str())) {
          frameEntity = entity;
        }

        if (entity == frameEntity) {
          frame = spatial.position;
        }
      }

      t += context->frameTime;

      auto view = context->registry->view<CameraController, Spatial>();
      
      view.each([&](auto &cameraController, auto &spatial){
        Vector3N direction({
          (int)context->inputBuffer->right - (int)context->inputBuffer->left,
          (int)context->inputBuffer->up - (int)context->inputBuffer->down,
          (int)context->inputBuffer->backward - (int)context->inputBuffer->forward
        });

        QuaternionF qConj = spatial.rotation.conjugate();
        QuaternionF qDir(0, direction.x, direction.y, direction.z);
        QuaternionF qGlobal = spatial.rotation * qDir * qConj;

        Vector3N globalDirection(qGlobal.i, qGlobal.j, qGlobal.k);

        // this only allows for a single camera controller, kinda janky
        camPos += globalDirection * cameraController.speed * context->frameTime;
        spatial.position = camPos + frame;

        if (context->inputBuffer->isFocused) {
          cameraController.pitch += 30 * context->inputBuffer->dy * context->frameTime;
          cameraController.yaw += 30 * context->inputBuffer->dx * context->frameTime;
        }

        cameraController.totalScroll += 2.f * (float)context->inputBuffer->scroll;

        cameraController.speed = std::exp(cameraController.totalScroll * 0.1) * 0.1;

        ImGui::Text("Camera Speed (KM/S): %f", cameraController.speed * 1e-3);

        // TODO: this is a dirty hack to fix mouse drifting
        // the problem occurs because mouse delta only updates
        // when the mouse moves, so it never goes to zero becasue
        // it isn't updated when the mouse is not moving
        context->inputBuffer->dy = 0;
        context->inputBuffer->dx = 0;

        context->inputBuffer->scroll = 0;

        //cameraController.pitch += 10 * context->frameTime;

        //printf("%f %f \n", cameraController.totalScroll, cameraController.speed);

        spatial.rotation = eulerToQuaternion(0, cameraController.yaw, cameraController.pitch);
      });
    }
};
