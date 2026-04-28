#pragma once

#include "core/math/matrix.hpp"
#include "core/resources/mesh.hpp"
#include "core/resources/material.hpp"
#include "core/handle.hpp"

struct DrawCommand {
  uint64_t key;
  uint8_t layer;
  Handle<Mesh> mesh;
  Handle<Material> material;
  Matrix4x4F transform;
};

struct CommandBuffer {
  private:
    std::vector<DrawCommand> commands;
  public:
    void submit(DrawCommand command) {
      commands.push_back(command);
    }

    void clear() {
      commands.clear();
    }

    void sort() {
      std::sort(
        commands.begin(),
        commands.end(),
        [](const DrawCommand& a, const DrawCommand& b) {
          return a.key < b.key;
        });
    }

    std::vector<DrawCommand> getCommands() const { return commands; }
};