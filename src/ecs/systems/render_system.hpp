#pragma once

#include "ecs/system.hpp"
#include "core/context.hpp"
#include "math/utilities.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/mesh_renderer.hpp"
#include "ecs/components/camera.hpp"

struct RenderSystem : System {
  void init() {

  }

  void draw() {
    // maybe right here i loop over all the cameras and preform the transforms into their
    // space ?
    // or maybe the spatial should stay as Number until a specific camera is rendering it?

    auto meshRendererView = context->registry->view<MeshRenderer, Spatial>();
    meshRendererView.each([this](auto &meshRenderer, auto &spatial){
      // need to determine culling
      context->renderer->addMesh3D(meshRenderer.mesh, meshRenderer.material, spatial);
    });

    auto cameraView = context->registry->view<Camera, Spatial>();
    cameraView.each([this](auto &camera, auto &spatial){
      context->renderer->addCamera3D(camera, spatial);
    });

    auto pointLightView = context->registry->view<PointLight, Spatial>();
    pointLightView.each([this](auto &light, auto &spatial){
      context->renderer->addPointLight(light, spatial);
    });
  }
};
