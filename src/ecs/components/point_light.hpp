#include "core/math/color.hpp"

// requires Spatial, position defined by the spatial
struct PointLight {
  float intensity; // w/m^2
  float range; // m
  Color color;
};
