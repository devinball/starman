#pragma once

#include "math/number.hpp"

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

    Vector2T& operator+=(const Vector2T& b) {
        this->x += b.x;
        this->y += b.y;
        return *this;
    }

    Vector2T& operator-=(const Vector2T& b) {
        this->x -= b.x;
        this->y -= b.y;
        return *this;
    }

    bool operator==(const Vector2T& b) {
        return this->x == b.x && this->y == b.y;
    }

    bool operator!=(const Vector2T& b) {
        return !(this->x == b.x && this->y == b.y);
    }

    Vector2T normalized() {
        return Vector2T{x, y} / magnitude();
    }

    T dot(const Vector2T &b) {
        return x * b.x + y * b.y;
    }
    
    T magnitude() {
        return sqrt(squareMagnitude());
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

    Vector3T& operator+=(const Vector3T& b) {
        this->x += b.x;
        this->y += b.y;
        this->z += b.z;
        return *this;
    }

    Vector3T& operator-=(const Vector3T& b) {
        this->x -= b.x;
        this->y -= b.y;
        this->z -= b.z;
        return *this;
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
        return sqrt(squareMagnitude());
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

    Vector4T& operator+=(const Vector4T& b) {
        this->x += b.x;
        this->y += b.y;
        this->z += b.z;
        this->w += b.w;
        return *this;
    }

    Vector4T& operator-=(const Vector4T& b) {
        this->x -= b.x;
        this->y -= b.y;
        this->z -= b.z;
        this->w -= b.w;
        return *this;
    }

    Vector4T normalized() {
        return Vector4T{x, y, z, w} / magnitude();
    }

    T dot(const Vector4T &b) {
        return x * b.x + y * b.y + z * b.z + w * b.w;
    }

    T magnitude() {
        return sqrt(squareMagnitude());
    }

    T squareMagnitude() {
        return x*x + y*y + z*z + w*w;
    }
};

using Vector2N = Vector2T<Number>;
using Vector3N = Vector3T<Number>;
using Vector4N = Vector4T<Number>;

using Vector2F = Vector2T<float>;
using Vector3F = Vector3T<float>;
using Vector4F = Vector4T<float>;

using Vector2D = Vector2T<double>;
using Vector3D = Vector3T<double>;
using Vector4D = Vector4T<double>;

using Vector2I = Vector2T<int>;
using Vector3I = Vector3T<int>;
using Vector4I = Vector4T<int>;

Vector3F toVector3F(Vector3N v) {
    return Vector3F{(float)v.x, (float)v.y, (float)v.z};
}

Vector3D toVector3D(Vector3N v) {
    return Vector3D{(double)v.x, (double)v.y, (double)v.z};
}

Vector3N toVector3N(Vector3F v) {
    return Vector3N{v.x, v.y, v.z};
}

/*
    // TODO: this
    // is D better ?? Or L?? or C??
    template <typename C>
    Vector4T<C> operator C() {
        return Vector4T<C>{this->x, this->y, this->z, this->w};
    }

*/
