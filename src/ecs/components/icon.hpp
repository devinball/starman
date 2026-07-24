#pragma once

#include "math/vector.hpp"
#include "math/color.hpp"

#include <vector>

struct Icon {
    Color color;
    std::vector<Vector3N> trajectory;
};