#pragma once

#include "ecs/system.hpp"
#include "core/context.hpp"
#include "core/math/utilities.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/mesh_renderer.hpp"
#include "ecs/components/camera.hpp"

struct RenderSystem : System {
  private:
    void localizeMesh(); // convert from Number space to local float

  public:
    void init() {
            // TODO: use camera properties
      auto cameraView = context->registry->view<Camera, Spatial>();
      cameraView.each([this](auto &camera, auto &spatial){
        context->sceneGraph->submitCamera(
          0, 0, 90.f, true,
          Color{0, 0, 0.2, 1.0},
          spatial.position,
          spatial.scale,
          spatial.rotation
        );
      });
      
    }

    void draw() {
      // maybe right here i loop over all the cameras and preform the transforms into their
      // space ?
      // or maybe the spatial should stay as Number until a specific camera is rendering it?
      
      context->sceneGraph->clear();

      // need to determine culling
      auto meshRendererView = context->registry->view<MeshRenderer, Spatial>();
      meshRendererView.each([this](auto &meshRenderer, auto &spatial){
        // must have uniform non-negative scaling
        // in the future allowing non-uniform scaling would be a good idea
        assert(spatial.scale.x == spatial.scale.y == spatial.scale.z);
        assert(spatial.scale.x > 0);

        // right here we need to preform a floating origin such
        // that the camera is always the renderer's origin
        // this way we can keep renderer performance, (maybe
        // even allowing me to use float16), while also
        // doing physics in large scale coordinates

        context->sceneGraph->submitModel(
          meshRenderer.mesh,
          meshRenderer.material,
          spatial.position,
          spatial.scale,
          spatial.rotation
        );
      });


    }
};
