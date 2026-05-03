#pragma once

#include "core/context.hpp"
#include "ecs/ecs.hpp"
#include "core/layers/layer.hpp"
#include "core/layers/project/project_layer.hpp"
#include "core/layers/resource/resource_layer.hpp"
#include "core/layers/render/render_layer.hpp"
#include "core/layers/scene/scene_layer.hpp"

#include <functional>
#include <vector>
#include <memory>

struct Stack {
  private:
    std::vector<std::unique_ptr<Layer>> layers;
  public:
    template <class T> 
    void add() {
      // TODO: throws an error if multiple of the same type are added
      std::unique_ptr<Layer> layer = std::make_unique<T>();
      layers.push_back(std::move(layer));
    }

    void each(std::function<void(Layer*)> f) {
      for (const auto& layer : layers) {
        f(layer.get());
      }
    }
};

struct Application {
  std::shared_ptr<float> dt;
  std::shared_ptr<Stack> stack;
  std::shared_ptr<SceneGraph> sceneGraph;
  std::shared_ptr<ResourcePool> resourcePool;
  std::shared_ptr<Registry> registry;

  // application holds onto layers
  // we can add and remove layers from the stack
  // application will by default add a bunch of layers
  // to the stack-> maybe in future this can be disabled
  // via build flags.

  bool running = true;

  void run() {
    stack = std::make_shared<Stack>();
    sceneGraph = std::make_shared<SceneGraph>();
    resourcePool = std::make_shared<ResourcePool>();
    registry = std::make_shared<Registry>();

    //systemRegistry.add<RenderSystem>();
    //systemRegistry.init(context);

    //context.dispatcher.sink<QuitEvent>().connect<&Application::quit>(this);

    //stack->add<ProjectLayer>();
    //stack->add<ResourceLayer>();
    stack->add<RenderLayer>();
    stack->add<SceneLayer>();

    stack->each([this](Layer *layer) {
      layer->context = { dt, sceneGraph, resourcePool, registry }; // send a context of useful things for event dispatching and buffers
    });

    stack->each([](Layer *layer) {
      layer->init();
    });

    while (running) {
      // start main loop
      stack->each([](Layer *layer) {
        layer->update();
      });

      // start render loop
      stack->each([](Layer *layer) {
        layer->draw();
      });
    }

    stack->each([](Layer *layer) {
      layer->shutdown();
    });
  }

  void dispatchEvent(Event event) {
    stack->each([event](Layer *layer) {
      layer->event(event);
    });
  }

  void quit(const QuitEvent&) {
    running = false;
  }
};
