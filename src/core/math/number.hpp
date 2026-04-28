#pragma once

// this file abstracts away details of multi precision numbers
// Numbers can be used just like normal numbers, with math functions and whatnot

#include <boost/multiprecision/cpp_bin_float.hpp>

using Number = boost::multiprecision::cpp_bin_float_50; // probably too big

Number sin(Number x) {
    return boost::multiprecision::sin(x);
}

Number cos(Number x) {
    return boost::multiprecision::cos(x);
}

Number tan(Number x) {
    return boost::multiprecision::tan(x);
}

Number sqrt(Number x) {
    return boost::multiprecision::sqrt(x);
}

Number pow(Number a, Number b) {
    return boost::multiprecision::pow(a, b);
}
