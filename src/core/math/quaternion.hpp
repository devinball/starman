#pragma once

#include "core/math/number.hpp"

// --- LLM WRITTEN ---
template <typename T>
struct QuaternionT {
    T r;  // real/scalar component
    T i;  // i component
    T j;  // j component
    T k;  // k component

    // Constructors
    QuaternionT() : r(1), i(0), j(0), k(0) {}  // Identity quaternion
    QuaternionT(T r, T i, T j, T k) : r(r), i(i), j(j), k(k) {}

    // Basic operations
    QuaternionT operator+(const QuaternionT &b) const {
        return QuaternionT(r + b.r, i + b.i, j + b.j, k + b.k);
    }

    QuaternionT operator-(const QuaternionT &b) const {
        return QuaternionT(r - b.r, i - b.i, j - b.j, k - b.k);
    }

    QuaternionT operator/(const T &b) const {
        return QuaternionT(r / b, i / b, j / b, k / b);
    }

    QuaternionT operator*(const T &b) const {
        return QuaternionT(r * b, i * b, j * b, k * b);
    }

    // Quaternion multiplication (Hamilton product)
    QuaternionT operator*(const QuaternionT &b) const {
        return QuaternionT(
            r * b.r - i * b.i - j * b.j - k * b.k,
            r * b.i + i * b.r + j * b.k - k * b.j,
            r * b.j - i * b.k + j * b.r + k * b.i,
            r * b.k + i * b.j - j * b.i + k * b.r
        );
    }

    // Multiply by vector (treating vector as pure quaternion with w=0)
    QuaternionT operator*(const Vector3 &b) const {
        return *this * QuaternionT(0, b.x, b.y, b.z);
    }

    // Norm (magnitude)
    T norm() const {
        return std::sqrt(r * r + i * i + j * j + k * k);
    }

    // Normalize
    QuaternionT normalized() const {
        T n = norm();
        return *this / n;
    }

    // Conjugate
    QuaternionT conjugate() const {
        return QuaternionT(r, -i, -j, -k);
    }

    // Inverse
    QuaternionT inverse() const {
        T n_squared = r * r + i * i + j * j + k * k;
        return conjugate() / n_squared;
    }

    // Dot product
    T dot(const QuaternionT &b) const {
        return r * b.r + i * b.i + j * b.j + k * b.k;
    }
};

using Quaternion = QuaternionT<Number>;
using QuaternionF = QuaternionT<float>;

QuaternionF euler_angles_to_quat(float pitch, float yaw, float roll) {
    // Abbreviations for the various angular functions
    float cy = std::cos(yaw * 0.5f);
    float sy = std::sin(yaw * 0.5f);
    float cp = std::cos(pitch * 0.5f);
    float sp = std::sin(pitch * 0.5f);
    float cr = std::cos(roll * 0.5f);
    float sr = std::sin(roll * 0.5f);

    return QuaternionF(
        cr * cp * cy + sr * sp * sy,
        sr * cp * cy - cr * sp * sy,
        cr * sp * cy + sr * cp * sy,
        cr * cp * sy - sr * sp * cy
    );
}