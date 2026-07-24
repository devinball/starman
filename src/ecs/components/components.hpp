// this might be worth it, yes it is a little dumb

#pragma once

#include "math/vector.hpp"
#include "ecs/ecs.hpp"

#include "resources/mesh.hpp"
#include "resources/material.hpp"
#include "resources/resource.hpp"

#include "math/number.hpp"
#include "math/vector.hpp"
#include "math/quaternion.hpp"

struct CameraController {
  float speed;
  float pitch;
  float yaw;
};

struct DirectionalLight {
  float intensity;
  Color color;
};

struct KeplerianOrbiter {
  double apoapsis;
  double periapsis;
  double eccentricity;
  double semiMajorAxis;
  double semiMinorAxis;
  double semiParameter;

  double inclination;
  double longAscendingNode;
  double argumentPeriapsis;
  double longPeriapsis;

  double meanMotion;
  double orbitalPeriod;

  Entity centerBody;
  Vector3N barycenter;
};

struct MeshRenderer {
  Handle<Mesh> mesh;
  Handle<Material> material;
};

struct NewtonianOrbiter {
  // i don't know if anything is actually needed
  // here, the system does all the work
};

struct PointLight {
  float intensity; // w/m^2
  float range; // m
  Color color;
};


using Trajectory = std::vector<Vector4>;

struct Rigidbody {
    //Number properTime;
    float mass;

    Vector3N velocity; // Should not need high precision, highest value is c anyways
    Vector3N angularVelocity; // maybe should be vector3
    Matrix3x3F momentInertia;
//    Number timeStep; // does not always have to be fixed maybe?
//    uint64_t lastPositionIndex; // index of the last position we reached
//    Trajectory trajectory; // list of 4-vectors that represents an objects path through spacetime
};

struct Spatial {
    Vector3N position;
    Vector3F scale;
    QuaternionF rotation;
};

struct Tag {
  std::string name;
  std::unordered_set<std::string> groups;
};
