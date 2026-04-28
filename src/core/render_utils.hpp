#pragma once

#include <vector>
#include <inttypes.h>
#include <string>

struct WindowConfig
{
  int width = 1280;
  int height = 720;
  const char *title = "App";
  bool vsync = true;
  bool resizable = true;
};

struct RenderTarget {
  uint32_t id = 0;
  bool isValid() const { return id != 0; }
  bool operator==(const RenderTarget&) const = default;
};

struct EvictionList
{
  std::vector<std::string> meshIds;
  std::vector<std::string> imageIds;
  std::vector<std::string> shaderIds;

  bool empty() const
  {
    return meshIds.empty() && imageIds.empty() && shaderIds.empty();
  }
};