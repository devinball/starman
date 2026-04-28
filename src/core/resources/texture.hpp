#pragma once

#include "core/resources/resource.hpp"

enum struct TextureFormat {
  rgb8,
  rgba8,
};

struct Texture : Resource {
  TextureFormat format;
  int width;
  int height;  
};

// why not?
struct Texture3 : Resource {
    
};