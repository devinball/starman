#pragma once

// this file abstracts away details of multi precision numbers
// Numbers can be used just like normal numbers, with math functions and whatnot

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <stdint.h>

using Number = boost::multiprecision::cpp_bin_float_50; // probably too big

/*
template <int W, int F>
struct FixedPoint {
    int64_t whole;
    int16_t scale = 2^F;

    // value = whole / scale;

    FixedPoint operator+(const FixedPoint& b) {
        if (scale == b.scale) {
            return FixedPoint(whole + b.whole, scale);
        }
    }

    FixedPoint operator-(const FixedPoint& b) {
        
    }

    FixedPoint operator*(const FixedPoint& b) {
        return FixedPoint
    }

    FixedPoint operator/(const FixedPoint& b) {
        
    }

    FixedPoint operator+() {

    }

    FixedPoint operator-() {
        
    }

    FixedPoint operator*() {

    }

    FixedPoint operator/() {
        
    }
};
*/

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
