#pragma once

#include "core/math/number.hpp"

// TODO: implement +=, -=, /=, *=, ==, etc

template <typename T>
struct Vector2T {
    T x;
    T y;

    Vector2T operator+(const Vector2T &b) {
        return Vector2T{x + b.x, y + b.y};
    }

    Vector2T operator-(const Vector2T &b) {
        return Vector2T{x - b.x, y - b.y};
    }

    Vector2T operator/(const T &b) {
        return Vector2T{x / b, y / b};
    }

    Vector2T operator*(const T &b) {
        return Vector2T{x * b, y * b};
    }

    Vector2T normalized() {
        return Vector2T{x, y} / magnitude();
    }

    T dot(const Vector2T &b) {
        return x * b.x + y * b.y;
    }
    
    T magnitude() {
        return sqrt(x*x + y*y);
    }

    T squareMagnitude() {
        return x*x + y*y;
    }
};

template <typename T>
struct Vector3T {
    T x;
    T y;
    T z;

    Vector3T operator+(const Vector3T &b) {
        return Vector3T{x + b.x, y + b.y, z + b.z};
    }

    Vector3T operator-(const Vector3T &b) {
        return Vector3T{x - b.x, y - b.y, z - b.z};
    }

    Vector3T operator/(const T &b) {
        return Vector3T{x / b, y / b, z / b};
    }

    Vector3T operator*(const T &b) {
        return Vector3T{x * b, y * b, z * b};
    }

    Vector3T normalized() {
        return Vector3T{x, y, z} / magnitude();
    }

    Vector3T cross(const Vector3T &b) {
        return Vector3T{y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x};
    }

    T dot(const Vector3T &b) {
        return x * b.x + y * b.y + z * b.z;
    }

    T magnitude() {
        return sqrt(x*x + y*y + z*z);
    }

    T squareMagnitude() {
        return x*x + y*y + z*z;
    }
};

template <typename T>
struct Vector4T {
    T x;
    T y;
    T z;
    T w;

    Vector4T operator+(const Vector4T &b) {
        return Vector4T{x + b.x, y + b.y, z + b.z, w + b.w};
    }

    Vector4T operator-(const Vector4T &b) {
        return Vector4T{x - b.x, y - b.y, z - b.z, w + b.w};
    }

    Vector4T operator/(const T &b) {
        return Vector4T{x / b, y / b, z / b, w / b};
    }

    Vector4T operator*(const T &b) {
        return Vector4T{x * b, y * b, z * b, w * b};
    }

    Vector4T normalized() {
        return Vector4T{x, y, z, w} / magnitude();
    }

    T dot(const Vector4T &b) {
        return x * b.x + y * b.y + z * b.z + w * b.w;
    }

    T magnitude() {
        return sqrt(x*x + y*y + z*z + w*w);
    }

    T squareMagnitude() {
        return x*x + y*y + z*z + w*w;
    }
};

using Vector2 = Vector2T<Number>;
using Vector3 = Vector3T<Number>;
using Vector4 = Vector4T<Number>;

using Vector2F = Vector2T<float>;
using Vector3F = Vector3T<float>;
using Vector4F = Vector4T<float>;

using Vector2I = Vector2T<int>;
using Vector3I = Vector3T<int>;
using Vector4I = Vector4T<int>;
