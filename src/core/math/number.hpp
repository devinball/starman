#pragma once


/*
// this file abstracts away details of multi precision numbers
// Numbers can be used just like normal numbers, with math functions and whatnot

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <stdint.h>

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
*/

/*
template <int F = 16>
struct FixedPoint {
    int64_t value;

    explicit FixedPoint(int64_t intPart = 0) : value(intPart << F) {}
    
    explicit FixedPoint(double floatVal) {
        value = static_cast<int16_t>(floatVal * (1 << F) + 0.5);
    }

    operator double() const {
        return static_cast<double>(value) / (1 << F);
    }

    operator float() const {
        return static_cast<float>(static_cast<double>(value) / (1 << F));
    }

    operator int() const {
        return value >> F;
    }

    FixedPoint operator+(const FixedPoint& b) {
        FixedPoint c;
        c.value = this->value + b.value;
        return c;
    }

    FixedPoint operator-(const FixedPoint& b) {
        FixedPoint c;
        c.value = this->value - b.value;
        return c;
    }

    FixedPoint operator*(const FixedPoint& b) {
        FixedPoint c;
        // TODO: should cast to 128 bit for this
        c.value = (this-value << F) * b.value;
        return c;
    }

    FixedPoint operator/(const FixedPoint& b) {
        FixedPoint c;
        c.value = (this-value << F) / b.value;
        return c;
    }

};
*/

template <int F = 16>
struct FixedPoint {
    int64_t value;

    FixedPoint() : value(0) {}

    FixedPoint(double floatVal) 
        : value(static_cast<int64_t>(floatVal * (1LL << F) + 0.5)) {}
    
    FixedPoint(float floatVal) 
        : value(static_cast<int64_t>(floatVal * (1LL << F) + 0.5f)) {}

    FixedPoint(int intVal) : value(static_cast<int64_t>(intVal) << F) {}

    FixedPoint(long int intVal) : value(static_cast<int64_t>(intVal) << F) {}

    // Conversions
    operator double() const {
        return static_cast<double>(value) / (1LL << F);
    }

    operator float() const {
        return static_cast<float>(value) / (1LL << F);
    }

    operator int() const {
        return static_cast<int>(value >> F);
    }

    // FixedPoint to FixedPoint operators
    FixedPoint operator+(const FixedPoint& b) const {
        FixedPoint c;
        c.value = this->value + b.value;
        return c;
    }

    FixedPoint operator-(const FixedPoint& b) const {
        FixedPoint c;
        c.value = this->value - b.value;
        return c;
    }

    FixedPoint operator*(const FixedPoint& b) const {
        FixedPoint c;
        c.value = static_cast<int64_t>(
            (__int128)this->value * b.value >> F
        );
        return c;
    }

    FixedPoint operator/(const FixedPoint& b) const {
        FixedPoint c;
        c.value = static_cast<int64_t>(
            ((__int128)this->value << F) / b.value
        );
        return c;
    }

    // FixedPoint op double
    FixedPoint operator+(double b) const { return *this + FixedPoint(b); }
    FixedPoint operator-(double b) const { return *this - FixedPoint(b); }
    FixedPoint operator*(double b) const { return *this * FixedPoint(b); }
    FixedPoint operator/(double b) const { return *this / FixedPoint(b); }

    // FixedPoint op float
    FixedPoint operator+(float b) const { return *this + FixedPoint(b); }
    FixedPoint operator-(float b) const { return *this - FixedPoint(b); }
    FixedPoint operator*(float b) const { return *this * FixedPoint(b); }
    FixedPoint operator/(float b) const { return *this / FixedPoint(b); }

    // FixedPoint op int
    FixedPoint operator+(int b) const { return *this + FixedPoint(b); }
    FixedPoint operator-(int b) const { return *this - FixedPoint(b); }
    FixedPoint operator*(int b) const { return *this * FixedPoint(b); }
    FixedPoint operator/(int b) const { return *this / FixedPoint(b); }
};

// double op FixedPoint (reverse operations)
template <int F>
inline FixedPoint<F> operator+(double a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) + b; 
}

template <int F>
inline FixedPoint<F> operator-(double a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) - b; 
}

template <int F>
inline FixedPoint<F> operator*(double a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) * b; 
}

template <int F>
inline FixedPoint<F> operator/(double a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) / b; 
}

// float op FixedPoint (reverse operations)
template <int F>
inline FixedPoint<F> operator+(float a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) + b; 
}

template <int F>
inline FixedPoint<F> operator-(float a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) - b; 
}

template <int F>
inline FixedPoint<F> operator*(float a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) * b; 
}

template <int F>
inline FixedPoint<F> operator/(float a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) / b; 
}

// int op FixedPoint (reverse operations)
template <int F>
inline FixedPoint<F> operator+(int a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) + b; 
}

template <int F>
inline FixedPoint<F> operator-(int a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) - b; 
}

template <int F>
inline FixedPoint<F> operator*(int a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) * b; 
}

template <int F>
inline FixedPoint<F> operator/(int a, const FixedPoint<F>& b) { 
    return FixedPoint<F>(a) / b; 
}

using Number = FixedPoint<16>;
