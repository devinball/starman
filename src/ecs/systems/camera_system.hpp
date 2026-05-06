#pragma once

#include "ecs/system.hpp"
#include "core/context.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/camera_controller.hpp"

struct CameraSystem : System {
  private:
    void localizeMesh(); // convert from Number space to local float

    float t;
  public:
    void init() {
      
    }

    void update() {
      t += context->frameTime;
      auto cameraView = context->registry->view<Camera>();
      auto cameraControllerView = context->registry->view<Camera, Spatial>();
      
      cameraView.each([this](auto &camera){
        // idk
      });

      cameraControllerView.each([this](auto &cameraController, auto &spatial){
        Vector3 direction({1, 1, 1});
        spatial.position += direction * cameraController.speed;
      });
    }

    void draw() {
      
    }
};
