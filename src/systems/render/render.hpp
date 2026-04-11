#pragma once

#include <entt/entt.hpp>
#include "core/events.hpp"
#include "ecs/ecs.hpp"
#include "core/context.hpp"

#include "systems/render/raylib/raylib.hpp"

#include <xcb/xcb.h>

struct RenderSystem : System {
  private:
    RenderContext renderContext;

    void localizeMesh(); // convert from BigNumber space to local float

  public:
    void init(Context& context) {
      renderContext.init(1280, 720, "engine");

      renderContext.setCameraPosition({10, 3, 3});
      renderContext.setCameraTarget({0, 0, 0});
    }

    void draw(Context& context, float dt) {
      renderContext.update();
      
      if (false) {
        renderContext.shutdown();
        context.dispatcher.trigger<QuitEvent>();
      }
      
      // auto view = registry.view<MeshRenderer, Spatial>();
      // view.each([](auto &mesh_renderer, auto &spatial){
        // interface with the graphics library and render the mesh
      // });
    }
};
