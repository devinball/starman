#pragma once

#include <entt/entt.hpp>
#include "core/events.hpp"
#include "ecs/ecs.hpp"
#include "systems/render/sdl/sdl.hpp"
#include "core/context.hpp"

struct RenderSystem : System {
  SDLContext sdlContext;

  void init(Context& context) {
    sdlContext = createSDLContext();
  }

  void draw(Context& context, float dt) {
    bool quit = pollSDLEvents(sdlContext);

    if (quit == true) {
      context.dispatcher.trigger<QuitEvent>();
    }
    // auto view = registry.view<MeshRenderer, Spatial>();
    // view.each([](auto &mesh_renderer, auto &spatial){
      // interface with the graphics library and render the mesh
    // });
  }
};
