#pragma once

#include "core/math/number.hpp"
#include <array>
#include <cmath>

template <typename T, uint8_t N, uint8_t M>
struct MatrixT
{
  std::array<T, N * M> data;

  uint8_t rows = N;
  uint8_t columns = M;

  MatrixT()
  {
    for (uint16_t i = 0; i < N * M; ++i)
    {
      data[i] = 0;
    }
  }

  MatrixT(std::array<T, N * M> initData)
  {
    data = initData;
  }

  MatrixT(T initData[N * M])
  {
    data = initData;
  }

  static MatrixT identity()
  {
    static_assert(N == M, "Identity matrix must be square");
    MatrixT m;
    for (uint16_t i = 0; i < N; ++i)
    {
      m.data[i * N + i] = 1;
    }
    return m;
  }

  static MatrixT zero()
  {
    return MatrixT();
  }

  static MatrixT one()
  {
    MatrixT m;
    for (uint16_t i = 0; i < N * M; ++i)
    {
      m.data[i] = 1;
    }
    return m;
  }

  // Element access
  T &operator()(uint8_t row, uint8_t col)
  {
    assert(row < rows && col < columns && "Matrix indices out of bounds");
    return data[row * columns + col];
  }

  const T &operator()(uint8_t row, uint8_t col) const
  {
    assert(row < rows && col < columns && "Matrix indices out of bounds");
    return data[row * columns + col];
  }

  std::array<T, N * M> getData() const {
    return data;
  }

  // Matrix addition
  MatrixT operator+(const MatrixT &b) const
  {
    assert(rows == b.rows && columns == b.columns && "Matrices must be of same size to add");
    MatrixT result;
    for (uint16_t i = 0; i < N * M; ++i)
    {
      result.data[i] = data[i] + b.data[i];
    }
    return result;
  }

  // Matrix subtraction
  MatrixT operator-(const MatrixT &b) const
  {
    assert(rows == b.rows && columns == b.columns && "Matrices must be of same size to subtract");
    MatrixT result;
    for (uint16_t i = 0; i < N * M; ++i)
    {
      result.data[i] = data[i] - b.data[i];
    }
    return result;
  }

  // Matrix multiplication
  template <uint8_t P>
  MatrixT<T, N, P> operator*(const MatrixT<T, M, P> &b) const
  {
    assert(columns == b.rows && "Number of columns of first must equal number of rows in second");
    MatrixT<T, N, P> result;

    for (uint8_t i = 0; i < N; ++i)
    {
      for (uint8_t j = 0; j < P; ++j)
      {
        T sum = 0;
        for (uint8_t k = 0; k < M; ++k)
        {
          sum += (*this)(i, k) * b(k, j);
        }
        result(i, j) = sum;
      }
    }
    return result;
  }

  // Scalar multiplication
  MatrixT operator*(T scalar) const
  {
    MatrixT result;
    for (uint16_t i = 0; i < N * M; ++i)
    {
      result.data[i] = data[i] * scalar;
    }
    return result;
  }

  // Scalar division
  MatrixT operator/(T scalar) const
  {
    assert(scalar != 0 && "Division by zero");
    MatrixT result;
    for (uint16_t i = 0; i < N * M; ++i)
    {
      result.data[i] = data[i] / scalar;
    }
    return result;
  }

  // Matrix equality
  bool operator==(const MatrixT &other) const
  {
    for (uint16_t i = 0; i < N * M; ++i)
    {
      if (data[i] != other.data[i])
      {
        return false;
      }
    }
    return true;
  }

  // Matrix inequality
  bool operator!=(const MatrixT &other) const
  {
    return !(*this == other);
  }

  // Transpose
  MatrixT<T, M, N> transpose() const
  {
    MatrixT<T, M, N> result;
    for (uint8_t i = 0; i < N; ++i)
    {
      for (uint8_t j = 0; j < M; ++j)
      {
        result(j, i) = (*this)(i, j);
      }
    }
    return result;
  }

  // Determinant (for square matrices)
  T determinant() const
  {
    static_assert(N == M, "Determinant requires square matrix");
    if constexpr (N == 1)
    {
      return data[0];
    }
    else if constexpr (N == 2)
    {
      return data[0] * data[3] - data[1] * data[2];
    }
    else if constexpr (N == 3)
    {
      return data[0] * (data[4] * data[8] - data[5] * data[7]) -
             data[1] * (data[3] * data[8] - data[5] * data[6]) +
             data[2] * (data[3] * data[7] - data[4] * data[6]);
    }
    else
    {
      // For larger matrices, use Laplace expansion (recursive)
      T det = 0;
      for (uint8_t col = 0; col < N; ++col)
      {
        MatrixT<T, N - 1, N - 1> submatrix;
        for (uint8_t i = 1; i < N; ++i)
        {
          uint8_t sub_col = 0;
          for (uint8_t j = 0; j < N; ++j)
          {
            if (j == col)
              continue;
            submatrix((i - 1), sub_col) = (*this)(i, j);
            sub_col++;
          }
        }
        T sign = (col % 2 == 0) ? 1 : -1;
        det += sign * (*this)(0, col) * submatrix.determinant();
      }
      return det;
    }
  }

  // Inverse (for square matrices)
  MatrixT inverse() const
  {
    static_assert(N == M, "Inverse requires square matrix");
    T det = determinant();
    assert(det != 0 && "Matrix is not invertible (determinant is zero)");

    if constexpr (N == 1)
    {
      MatrixT result;
      result(0, 0) = 1 / data[0];
      return result;
    }
    else if constexpr (N == 2)
    {
      MatrixT result;
      result(0, 0) = data[3] / det;
      result(0, 1) = -data[1] / det;
      result(1, 0) = -data[2] / det;
      result(1, 1) = data[0] / det;
      return result;
    }
    else
    {
      // For larger matrices, use adjugate matrix method
      MatrixT adjugate;
      for (uint8_t i = 0; i < N; ++i)
      {
        for (uint8_t j = 0; j < N; ++j)
        {
          MatrixT<T, N - 1, N - 1> submatrix;
          uint8_t sub_i = 0;
          for (uint8_t ii = 0; ii < N; ++ii)
          {
            if (ii == i)
              continue;
            uint8_t sub_j = 0;
            for (uint8_t jj = 0; jj < N; ++jj)
            {
              if (jj == j)
                continue;
              submatrix(sub_i, sub_j) = (*this)(ii, jj);
              sub_j++;
            }
            sub_i++;
          }
          T sign = ((i + j) % 2 == 0) ? 1 : -1;
          adjugate(j, i) = sign * submatrix.determinant();
        }
      }
      return adjugate / det;
    }
  }
};

using Matrix2x2 = MatrixT<Number, 2, 2>;
using Matrix2x2F = MatrixT<float, 2, 2>;

using Matrix3x3 = MatrixT<Number, 3, 3>;
using Matrix3x3F = MatrixT<float, 3, 3>;

using Matrix4x4 = MatrixT<Number, 4, 4>;
using Matrix4x4F = MatrixT<float, 4, 4>;




/* -- part i wrote, used as template to make llm complete version above
template <typename T, uint8_t N, uint8_t M>
struct MatrixT {
  std::array<T, N * M> data;

  uint8_t rows = N;
  uint8_t columns = M;

  MatrixT() {
    for (uint16_t i = 0; i < N * M; ++i) {
      data[i] = 0;
    }
  }

  static MatrixT identity() {
    MatrixT m;
    for (uint16_t i = 0; i < N * M; ++i) {
      m.data[i] = 0;
    }
    return m;
  }

  static MatrixT zero() {
    MatrixT m;
    for (uint16_t i = 0; i < N * M; ++i) {
      m.data[i] = 0;
    }
    return m;
  }

  static MatrixT one() {
    MatrixT m;
    for (uint16_t i = 0; i < N * M; ++i) {
      m.data[i] = 1;
    }
    return m;
  }

  MatrixT operator+(const MatrixT<T, M, N> &b) {
    assert(rows == b.rows && columns == b.columns && "Matrices must be of same size to add");
    for (uint16_t i = 0; i < N * M; ++i) {
      data[i] += b.data[i];
    }
  }

  MatrixT operator-(const MatrixT &b) {
    assert(rows == b.rows && columns == b.columns && "Matrices must be of same size to subtract");
    for (uint16_t i = 0; i < N * M; ++i) {
      data[i] += b.data[i];
    }
  }

  MatrixT<T, M, b.columns> operator*(const MatrixT &b) {
    assert(rows == b.columns && "Number of columns of first must equal number of rows in second");
  }

  MatrixT inverse() {

  }

  T determinate() {

  }
};
*/
