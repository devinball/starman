#include "core/math/color.hpp"

// requires Spatial, position defined by the spatial
struct PointLight {
  double intensity; // w/m^2
  double range; // m
  Color color;
};
