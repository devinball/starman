#pragma once

enum struct TextureFormat {
  rgb8,
  rgba8,
};

struct Texture2 {
  TextureFormat format;
  int width;
  int height;  
};

// why not?
struct Texture3 {
    
};