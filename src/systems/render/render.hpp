#pragma once

#include <entt/entt.hpp>
#include "core/events.hpp"
#include "ecs/ecs.hpp"
#include "core/context.hpp"

#include "systems/render/diligent/diligent.hpp"

#include <xcb/xcb.h>

struct RenderSystem : System {
  RenderContext renderContext;

  void init(Context& context) {
    renderContext.init(1280, 720, "engine");
  }

  void draw(Context& context, float dt) {
    if (!renderContext.update()) {
      context.dispatcher.trigger<QuitEvent>();
    }
    
    // auto view = registry.view<MeshRenderer, Spatial>();
    // view.each([](auto &mesh_renderer, auto &spatial){
      // interface with the graphics library and render the mesh
    // });
  }
};
