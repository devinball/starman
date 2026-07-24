#pragma once

using Number = double;

/*
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <type_traits>

struct Number {
  boost::multiprecision::float128 v;

  template<class T, class = std::enable_if_t<std::is_arithmetic_v<T>>;
  Number(T x) : v(x) {}

  operator boost_Number&() { return v; }
  operator const boost_Number&() const { return v; }
}
*/

/*
//todo, allow -X
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

    bool operator==(const FixedPoint& b) const { return this->value == b.value; }
    bool operator!=(const FixedPoint& b) const { return this->value != b.value; }
    bool operator<(const FixedPoint& b) const { return this->value < b.value; }
    bool operator<=(const FixedPoint& b) const { return this->value <= b.value; }
    bool operator>(const FixedPoint& b) const { return this->value > b.value; }
    bool operator>=(const FixedPoint& b) const { return this->value >= b.value; }

    // Compound assignment operators
    FixedPoint& operator+=(const FixedPoint& b) {
        this->value += b.value;
        return *this;
    }

    FixedPoint& operator-=(const FixedPoint& b) {
        this->value -= b.value;
        return *this;
    }

    FixedPoint& operator*=(const FixedPoint& b) {
        this->value = static_cast<int64_t>(
            (__int128)this->value * b.value >> F
        );
        return *this;
    }

    FixedPoint& operator/=(const FixedPoint& b) {
        this->value = static_cast<int64_t>(
            ((__int128)this->value << F) / b.value
        );
        return *this;
    }

    // Compound assignment with double
    FixedPoint& operator+=(double b) { return *this += FixedPoint(b); }
    FixedPoint& operator-=(double b) { return *this -= FixedPoint(b); }
    FixedPoint& operator*=(double b) { return *this *= FixedPoint(b); }
    FixedPoint& operator/=(double b) { return *this /= FixedPoint(b); }

    // Compound assignment with float
    FixedPoint& operator+=(float b) { return *this += FixedPoint(b); }
    FixedPoint& operator-=(float b) { return *this -= FixedPoint(b); }
    FixedPoint& operator*=(float b) { return *this *= FixedPoint(b); }
    FixedPoint& operator/=(float b) { return *this /= FixedPoint(b); }

    // Compound assignment with int
    FixedPoint& operator+=(int b) { return *this += FixedPoint(b); }
    FixedPoint& operator-=(int b) { return *this -= FixedPoint(b); }
    FixedPoint& operator*=(int b) { return *this *= FixedPoint(b); }
    FixedPoint& operator/=(int b) { return *this /= FixedPoint(b); }
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


// --- LLM WRITTEN --- 
template <int F>
FixedPoint<F> sqrt(FixedPoint<F> x) {
    if (x.value <= 0) {
        throw std::runtime_error("Expected positive value for sqrt");
    }

    int64_t guess = x.value;

    int bits = 64 - __builtin_clzll(x.value);
    guess = 1LL << ((bits + F) / 2);

    for (int i = 0; i < 8; ++i) {
        int64_t next = (guess + ((__int128)x.value << F) / guess) >> 1;

        if (next >= guess) break;

        guess = next;
    }

    FixedPoint<F> result;
    result.value = guess;
    return result;
}

using Number = FixedPoint<16>;
*/