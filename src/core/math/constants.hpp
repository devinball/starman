#pragma once

//#include <boost/multiprecision/cpp_bin_float.hpp>
#include "core/math/number.hpp"

namespace Constants {
  const Number pi(3.1415); //= boost::math::constants::pi<Number>(); // ratio of diameter to circumference
  const Number tau(6.28);// = boost::math::constants::tau<Number>(); // 2 times pi
  const Number e(2.718);// = boost::math::constants::e<Number>(); // euler's number
  const Number G(0.000000000066743015); // gravitational constant
  const Number c(299792458); // speed of light in a vacuum
  const Number c2(89875517873681764); // speed of light squared
}
