#pragma once

#include "core/resources/resource.hpp"

#include <vector>
#include <string>
#include "ecs/ecs.hpp"

// a scene is a fully indepentant collection of entities and systems
// technically it's a resource, but i don't care enough to make it go
// through the resource pool right now
struct Scene : Resource {
  private:
    struct EntityDescription {
      
    };
    std::vector<EntityDescription> entities;
  public:

    using Resource::Resource;

    std::vector<EntityDescription> getEntities() const {
      return entities;
    }

    bool load() {
      return true;
    }

    bool unload() {
      return false;
    }
};
