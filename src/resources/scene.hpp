#pragma once

#include "resources/resource.hpp"

#include <vector>
#include <string>
#include "ecs/ecs.hpp"

//#include "rfl.hpp"
//#include "rfl/yaml.hpp"

// a scene is a fully independent collection of entities and systems
// technically it's a resource, but i don't care enough to make it go
// through the resource pool right now
struct Scene : Resource {
  using Resource::Resource;

  //std::vector<EntityDescription> entities;

  //using Component = std::variant<Spatial, Rigidbody, Camera>; // etc
  //using Entity = std::vector<Component>;

  //struct Config {
  //  std::vector<Entity> entities;
  //};

  struct Description {
    //std::unordered_map>std::string, rfl::Generic> components;
  };

  std::vector<Description> entities;

  bool load() {
    //auto result = rfl::yaml::read<Config>(readFile(getId())).value();

    // simple example, hardcoded, scene

    auto d1 = Description{
      
    };

    entities.push_back(d1);

    return true;
  }

  bool unload() {
    return false;
  }
};
