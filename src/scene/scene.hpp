#include "vector";

// a scene is a fully indepentant collection of entities and systems
struct Scene {
  std::vector<Entity> entities;
};
