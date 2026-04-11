#pragma once

#include "core/context.hpp"
#include "ecs/ecs.hpp"
#include "systems/render/render.hpp"

struct Application {
  SystemRegistry systemRegistry;
  Context context;

  bool running = true;

  void run() {
    systemRegistry.add<RenderSystem>();
    systemRegistry.init(context);

    context.dispatcher.sink<QuitEvent>().connect<&Application::quit>(this);

    while (true) {
      // start main loop
      systemRegistry.update(context, 0.016f);
      // start render loop
      systemRegistry.draw(context, 0.016f);
    }

    systemRegistry.shutdown();
  }

  void quit(const QuitEvent&) {
    running = false;
  }
};
