#pragma once
// take a scene.toml file and load it into an actual scene
// a scene.toml can reference prefabs

#include "core/layers/layer.hpp"
#include "ecs/ecs.hpp"
#include "core/context.hpp"
#include "core/resources/scene.hpp"
#include "ecs/ecs.hpp"

#include "ecs/components/spatial.hpp"
#include "ecs/components/mesh_renderer.hpp"
#include "ecs/systems/render_system.hpp"

#include <memory>
#include <vector>

// you can have multiple scenes!, why not!
// each scene keeps track of what entities it 'owns'
// that's all a scene does though, tracks entities.

// maybe i could use the onEvent better
// an event could be a draw call, a resource load or get request
// it could be a scene load, or a window close

struct SceneLayer : Layer {
  private:
    std::vector<std::unique_ptr<System>> systems;
    std::vector<std::unique_ptr<Scene>> loadedScenes;
  public:
    void init() {
      // TODO: !!!! SERIOUS PERFORMANCE ISSUES WITH MANY MESHES
      int n = 10;
      for (int x = 0; x < n; ++x) {
        for (int y = 0; y < n; ++y) {
          Entity e = context.registry->create();
          context.registry->emplace<Spatial>(e, Vector3{2 * x - float(n) * 0.5, 0, 2 * y - float(n) * 0.5}, Vector3{1, 1, 1}, euler_angles_to_quat(x, y, 0));
          Handle<Mesh> mesh = context.resourcePool->load<Mesh>("example/models/frog.stl");
          Handle<Material> material = Handle<Material>();//context.resourcePool->load<Material>("example/models/frog.mat");
          context.registry->emplace<MeshRenderer>(e, mesh, material);
        }
      }
      addSystem<RenderSystem>();

      for (auto& system : systems) {
        system->context = context;
      }
      
      // load scenethis
      // init all systems from registry
      for (auto& system : systems) {
        system->init();
      }
    }

    void update() {
      // loop all systems from registry on fixed timestep
      for (auto& system : systems) {
        system->update();
      }
    }

    void draw() {
      // loop all systems from registry on every frame
      for (auto& system : systems) {
        system->draw();
      }
    }

    void shutdown() {

    }

    template<typename T, typename... Args>
    T& addSystem(Args&&... args) {
      auto& ptr = systems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
      return static_cast<T&>(*ptr);
    }

    void loadScene(Handle<Scene> sceneHandle) {
      /*
      Scene* scene = resourcePool->get(sceneHandle);
      if (scene == nullptr) return;

      auto& entities = scene->getEntities();

      for (auto& entityData : entities) {
        const auto entity = registry.create();
        registry.emplace<Spatial>(entity);

      }


      // load everything
      */
    }

    void unloadScene(const std::string &id) {

    }
};


/*
#pragma once

#include "core/layers/layer.hpp"
#include "ecs/ecs.hpp"
#include "ecs/system.hpp"
#include "core/context.hpp"

#include "ecs/systems/render_system.hpp"

#include <vector>
#include <memory>

struct SystemLayer : Layer {
  private:
    std::vector<std::unique_ptr<System>> systems;
    Context context;
  public:
    void setContext(Context ctx) {
      add<RenderSystem>();
      
      context = ctx;
      for (auto& system : systems) {
        system->setContext(ctx);
      }
    }

    template<typename T, typename... Args>
    T& add(Args&&... args) {
      auto& ptr = systems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
      return static_cast<T&>(*ptr);
    }

    void init() {
      // load scenethis
      // init all systems from registry
      for (auto& system : systems) {
        system->init();
      }
    }

    void update() {
      // loop all systems from registry on fixed timestep
      for (auto& system : systems) {
        system->update();
      }
    }

    void draw() {
      // loop all systems from registry on every frame
      for (auto& system : systems) {
        system->draw();
      }
    }

    void shutdown() {

    }
};

*/