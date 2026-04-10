#pragma once

// just a wrapper of EnTT
#include <vector>
#include <memory>

#include <entt/entt.hpp>

#include "core/context.hpp"

using Registry = entt::registry;

struct System {
  virtual void init(Context& context) {};
  virtual void update(Context& context, float dt) {};
  virtual void draw(Context& context, float dt) {};
};

struct SystemRegistry
{
  private:
    std::vector<std::unique_ptr<System>> systems;

  public:
    template<typename T, typename... Args>
    T& add(Args&&... args) {
      auto& ptr = systems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
      return static_cast<T&>(*ptr);
    }

    void init(Context& context) {
      // load scenethis
      // init all systems from registry
      for (auto& system : systems) {
        system->init(context);
      }
    }

    void update(Context& context, float dt) {
      // loop all systems from registry on fixed timestep
      for (auto& system : systems) {
        system->update(context, dt);
      }
    }

    void draw(Context& context, float dt) {
      // loop all systems from registry on every frame
      for (auto& system : systems) {
        system->draw(context, dt);
      }
    }

    void shutdown() {

    }
};
