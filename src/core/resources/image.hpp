#pragma once

#include "core/resources/resource.hpp"
#include "core/filesystem/image.hpp"

#include <stdint.h>
#include <vector>

enum class FilterMode {
  Linear,
  Nearest
};

enum class WrapMode 
{ 
  Repeat,
  Clamp,
  Mirror
};

struct Image : Resource {
  using Resource::Resource;

  int width;
  int height;
  int channels;
  int depth;
  unsigned char *data;
  // probably should be a byte array

  FilterMode filter  = FilterMode::Linear;
  WrapMode   wrapU   = WrapMode::Repeat;
  WrapMode   wrapV   = WrapMode::Repeat;
  bool       genMips = true;

  bool load() override {
    data = stbi_load(getId().c_str(), &width, &height, &channels, 0);

    if (data) {
      printf("Loaded Image: %s\n", getId().c_str());
      return true;
    }

    printf("uhoh a: %s\n", getId().c_str());
    
    return false;
  }

  bool unload() override {
    stbi_image_free(data);
    return true;
  }
};
