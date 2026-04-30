#pragma once

#include "core/resources/resource.hpp"

#include <string>

// shader does not contain any renderer specific data
// the renderer needs to keep track of it
struct Shader : Resource {
  std::string vertexSource;
  std::string fragmentSource;
  std::string geometrySource;

  uint shaderProgram;
  // eitehr this or a std::unordered_map<std::string, uint>
  // the map would hold id, shaderProgram
  // then you would reference a shader by id, the renderer
  // does all the translating

  bool load() override {
    return true;
  }

  bool unload() override {
    return false;
  }
};
