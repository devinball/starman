#pragma once

#include "core/resources/resource.hpp"

#include <stdint.h>
#include <vector>

enum class FilterMode { Linear, Nearest };
enum class WrapMode   { Repeat, Clamp, Mirror };

struct Image : Resource {
  using Resource::Resource;

  const uint16_t width = 0;
  const uint16_t height = 0;
  const uint8_t channels = 4;
  const uint8_t depth = 8;
  std::vector<uint8_t> pixels;

  FilterMode filter  = FilterMode::Linear;
  WrapMode   wrapU   = WrapMode::Repeat;
  WrapMode   wrapV   = WrapMode::Repeat;
  bool       genMips = true;

  bool load() override {
    return true;
  }

  bool unload() override {
    pixels.clear();
    return true;
  }
};
