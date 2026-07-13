#pragma once

#include "ecs/system.hpp"
#include "core/context.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/camera_controller.hpp"

#include "core/input_buffer.hpp"

#include <math.h>

struct CameraSystem : System {
  private:
    void localizeMesh(); // convert from Number space to local float

    float t;
    float totalScroll = 200;
  public:
    void update() {
      t += context->frameTime;
      auto view = context->registry->view<CameraController, Spatial>();
      
      view.each([this](auto &cameraController, auto &spatial){
        Vector3 direction({
          (int)context->inputBuffer->right - (int)context->inputBuffer->left,
          (int)context->inputBuffer->up - (int)context->inputBuffer->down,
          (int)context->inputBuffer->backward - (int)context->inputBuffer->forward
        });

        QuaternionF qConj = spatial.rotation.conjugate();
        QuaternionF qDir(0, direction.x, direction.y, direction.z);
        QuaternionF qGlobal = spatial.rotation * qDir * qConj;

        Vector3 globalDirection(qGlobal.i, qGlobal.j, qGlobal.k);

        spatial.position = spatial.position + globalDirection * cameraController.speed * context->dt;

        cameraController.pitch += 7.5 * context->inputBuffer->dy * context->dt;
        cameraController.yaw += 7.5 * context->inputBuffer->dx * context->dt;

        totalScroll += (float)context->inputBuffer->scroll;

        cameraController.speed = std::exp(totalScroll * 0.1) * 0.1;

        // TODO: this is a dirty hack to fix mouse drifting
        // the problem occurs because mouse delta only updates
        // when the mouse moves, so it never goes to zero becasue
        // it isn't updated when the mouse is not moving
        context->inputBuffer->dy = 0;
        context->inputBuffer->dx = 0;

        context->inputBuffer->scroll = 0;

        //cameraController.pitch += 10 * context->dt;

        //printf("%f %f \n", totalScroll, cameraController.speed);

        spatial.rotation = eulerToQuaternion(0, cameraController.yaw, cameraController.pitch);
      });
    }
};
