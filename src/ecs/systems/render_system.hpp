#pragma once

#include "ecs/system.hpp"
#include "core/command_buffer.hpp"
#include "core/context.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/mesh_renderer.hpp"

struct RenderSystem : System {
  private:
    void localizeMesh(); // convert from Number space to local float

    float t;
  public:
    void init() {
    }

    void update() {
    }

    void draw() {
      t += 0.01;
      auto view = context.registry->view<MeshRenderer, Spatial>();

      // need to determine culling

      context.sceneGraph->clear();

      // maybe right here i loop over all the cameras and preform the transforms into their
      // space ?

      // or maybe the spatial should stay as Number until a specific camera is rendering it?

      view.each([this](auto &mesh_renderer, auto &spatial){
        // must have uniform non-negative scaling
        // in the future allowing non-uniform scaling would be a good idea
        assert(spatial.scale.x == spatial.scale.y == spatial.scale.z);
        assert(spatial.scale.x > 0);

        // right here we need to preform a floating origin such
        // that the camera is always the renderer's origin
        // this way we can keep renderer performance, (maybe
        // even allowing me to use float16), while also
        // doing physics in large scale coordinates

        context.sceneGraph->submitModel(
          mesh_renderer.mesh,
          Handle<Material>(),
          transformMatrix(
            Vector3F{(float)spatial.position.x, (float)spatial.position.y, (float)spatial.position.z},
            Vector3F{(float)spatial.scale.x, (float)spatial.scale.y, (float)spatial.scale.z},
            eulerToQuat(t, t, t) //spatial.rotation
          )
        );
      });
    }
};
