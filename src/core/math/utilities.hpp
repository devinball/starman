#pragma once

#include "core/math/vector.hpp"
#include "core/math/matrix.hpp"
#include "core/math/quaternion.hpp"
#include "core/math/constants.hpp"

Matrix4x4F identityMatrix4x4F() {
  return Matrix4x4F({
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
  });
}

QuaternionF eulerToQuat(float pitch, float yaw, float roll)
{
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

// --- LLM WRITTEN ---
Matrix4x4F eulerToMatrix(float pitch, float yaw, float roll) {
    // Convert degrees to radians
    float pitchRad = pitch * M_PI / 180.0f;
    float yawRad = yaw * M_PI / 180.0f;
    float rollRad = roll * M_PI / 180.0f;

    // Compute sines and cosines
    float cx = cosf(pitchRad), sx = sinf(pitchRad);
    float cy = cosf(yawRad),   sy = sinf(yawRad);
    float cz = cosf(rollRad),  sz = sinf(rollRad);

    // Compute rotation matrix (XYZ order: pitch (X), yaw (Y), roll (Z))
    return Matrix4x4F({
        cy * cz,            -cy * sz,            sy,    0.0f,
        cz * sx * sy + cx * sz,  cx * cz - sx * sy * sz, -cy * sx, 0.0f,
        -cx * cz * sy + sx * sz, cz * sx + cx * sy * sz,  cx * cy,  0.0f,
        0.0f,               0.0f,               0.0f,   1.0f
});
}

Matrix4x4F quatToMatrix(QuaternionF q) {
  Matrix4x4F m1({
    q.k, q.j, -q.i, q.r,
    -q.j, q.k, q.r, q.i,
    q.i, -q.r, -q.k, q.j,
    -q.r, -q.i, -q.j, q.k
  });

  Matrix4x4F m2({
    q.k, q.j, -q.i, -q.r,
    -q.j, q.k, q.r, -q.i,
    q.i, -q.r, -q.k, -q.j,
    q.r, q.i, q.j, q.k
  });
  
  return m1 * m2;
}

Matrix4x4F transformMatrixExpanded(Vector3F position, Vector3F scale, QuaternionF rotation)
{
  Matrix4x4F positionMatrix({
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    position.x, position.y, position.z, 1,
  });

  Matrix4x4F scaleMatrix({
    scale.x, 0, 0, 0,
    0, scale.y, 0, 0,
    0, 0, scale.z, 0,
    0, 0, 0, 1,
  });

  Matrix4x4F m1({
    rotation.k, rotation.j, -rotation.i, rotation.r,
    -rotation.j, rotation.k, rotation.r, rotation.i,
    rotation.i, -rotation.r, -rotation.k, rotation.j,
    -rotation.r, -rotation.i, -rotation.j, rotation.k
  });

  Matrix4x4F m2({
    rotation.k, rotation.j, -rotation.i, -rotation.r,
    -rotation.j, rotation.k, rotation.r, -rotation.i,
    rotation.i, -rotation.r, -rotation.k, -rotation.j,
    rotation.r, rotation.i, rotation.j, rotation.k
  });
  
  Matrix4x4F rotationMatrix = m1 * m2;

  return rotationMatrix * scaleMatrix * positionMatrix;
}

// --- LLM WRITTEN ---
// this is just a multiplied out version of transformMatrixExpanded
// well, it's almost the same, it just rotates things 180 yaw
Matrix4x4F transformMatrix(Vector3F position, Vector3F scale, QuaternionF rotation)
{
  float mag = sqrt(rotation.r*rotation.r + rotation.i*rotation.i + rotation.j*rotation.j + rotation.k*rotation.k);
  rotation.r /= mag; rotation.i /= mag;
  rotation.j /= mag; rotation.k /= mag;

  float r2 = rotation.r * rotation.r;
  float i2 = rotation.i * rotation.i;
  float j2 = rotation.j * rotation.j;
  float k2 = rotation.k * rotation.k;
  
  float ri = rotation.r * rotation.i;
  float rj = rotation.r * rotation.j;
  float rk = rotation.r * rotation.k;
  float ij = rotation.i * rotation.j;
  float ik = rotation.i * rotation.k;
  float jk = rotation.j * rotation.k;

  return Matrix4x4F({
    scale.x * (r2 + i2 - j2 - k2),     scale.x * (2*(ij - rk)),           scale.x * (2*(ik + rj)),           0,
    scale.y * (2*(ij + rk)),           scale.y * (r2 - i2 + j2 - k2),     scale.y * (2*(jk - ri)),           0,
    scale.z * (2*(ik - rj)),           scale.z * (2*(jk + ri)),           scale.z * (r2 - i2 - j2 + k2),     0,
    position.x,                        position.y,                        position.z,                        1
  });
}

Number gamma(Vector3 relativeVelocity) {
  Number v2 = relativeVelocity.squareMagnitude();
  return 1 / (sqrt(1 - (v2 / Constants::c2)));
}
