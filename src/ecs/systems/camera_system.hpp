#pragma once

#include "ecs/system.hpp"
#include "core/context.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/camera_controller.hpp"

#include "core/input_buffer.hpp"

struct CameraSystem : System {
  private:
    void localizeMesh(); // convert from Number space to local float

    float t;
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

        cameraController.pitch += 10 * context->inputBuffer->dy * context->dt;
        cameraController.yaw += 10 * context->inputBuffer->dx * context->dt;

        //cameraController.pitch += 10 * context->dt;

        spatial.rotation = eulerToQuaternion(0, cameraController.yaw, cameraController.pitch);
      });
    }
};
