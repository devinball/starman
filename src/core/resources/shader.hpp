#pragma once

#include "core/resources/resource.hpp"

#include <string>

struct Shader : Resource {
  std::string vertexSrc;
  std::string fragmentSrc;
  std::string geometrySrc;

  bool load() override {
    return true;
  }
};
