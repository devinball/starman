#pragma once

namespace Raylib {
  #include "raylib.h"
  #include "rlgl.h"
}

#include "core/math/vector.hpp"
#include "core/resources/mesh.hpp"
#include "core/layers/render/renderer.hpp"
#include "core/resource_pool.hpp"

#include <vector>
#include <iostream>
#include <string>

Raylib::Mesh convertToRaylibMesh(const Mesh& mesh) {
    // Create a raylib mesh
    Raylib::Mesh raylibMesh = {};

    // Set the number of vertices and indices
    raylibMesh.vertexCount = static_cast<unsigned int>(mesh.vertices.size());
    raylibMesh.triangleCount = static_cast<unsigned int>(mesh.indices.size() / 3);

    // Allocate memory for vertices, normals, texcoords, and indices
    raylibMesh.vertices = new float[raylibMesh.vertexCount * 3];
    raylibMesh.normals = new float[raylibMesh.vertexCount * 3];
    raylibMesh.texcoords = new float[raylibMesh.vertexCount * 2];
    raylibMesh.indices = new unsigned short[raylibMesh.triangleCount * 3];

    // Copy vertex data
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        raylibMesh.vertices[i * 3] = mesh.vertices[i].x;
        raylibMesh.vertices[i * 3 + 1] = mesh.vertices[i].y;
        raylibMesh.vertices[i * 3 + 2] = mesh.vertices[i].z;
    }

    // Copy normal data
    for (size_t i = 0; i < mesh.normals.size(); ++i) {
        raylibMesh.normals[i * 3] = mesh.normals[i].x;
        raylibMesh.normals[i * 3 + 1] = mesh.normals[i].y;
        raylibMesh.normals[i * 3 + 2] = mesh.normals[i].z;
    }

    // Copy UV data
    for (size_t i = 0; i < mesh.uvs.size(); ++i) {
        raylibMesh.texcoords[i * 2] = mesh.uvs[i].x;
        raylibMesh.texcoords[i * 2 + 1] = mesh.uvs[i].y;
    }

    // Copy index data
    for (size_t i = 0; i < mesh.indices.size(); ++i) {
        raylibMesh.indices[i] = static_cast<unsigned int>(mesh.indices[i]);
    }

    return raylibMesh;
}

struct RaylibRenderer : Renderer {
  private:
    Raylib::Camera3D camera;

    std::unordered_map<std::string, Raylib::Mesh> meshCache;
  public:
    void init(int width, int height, char name[]) {
      Raylib::InitWindow(width + 1, height + 1, "engine");
      Raylib::SetWindowState(Raylib::FLAG_WINDOW_RESIZABLE);
      Raylib::SetWindowSize(width, height);
      Raylib::SetTargetFPS(60);

      camera.up = { 0, 1, 0 };
      camera.fovy = 45.0f;
      camera.projection = Raylib::CAMERA_PERSPECTIVE;
    }

    void beginFrame() {
      Raylib::BeginDrawing();
        Raylib::ClearBackground(Raylib::RAYWHITE);
        Raylib::BeginMode3D(camera);
    }

    void endFrame() {
      Raylib::EndMode3D();
      Raylib::DrawFPS(20, 20);
      Raylib::EndDrawing();
    }

    void executeCommands(std::vector<DrawCommand> drawCommands) {
      for (const auto& command : drawCommands) {
        if (meshCache.find(command.mesh.getId()) == meshCache.end()) {
          Mesh* mesh = resourcePool->get(command.mesh);
          Raylib::Mesh m = convertToRaylibMesh(*mesh);
          meshCache[command.mesh.getId()] = m;
          printf("a\n");
        }

        Raylib::Matrix transformMatrix = {
            command.transform(0, 0), command.transform(0, 1), command.transform(0, 2), command.transform(0, 3),
            command.transform(1, 0), command.transform(1, 1), command.transform(1, 2), command.transform(1, 3),
            command.transform(2, 0), command.transform(2, 1), command.transform(2, 2), command.transform(2, 3),
            command.transform(3, 0), command.transform(3, 1), command.transform(3, 2), command.transform(3, 3)
        };

        Raylib::Material mat = Raylib::LoadMaterialDefault();
        mat.maps->color = Raylib::GREEN;
        Raylib::DrawMesh(meshCache[command.mesh.getId()], mat, transformMatrix);
      }
    }

    void setCameraPosition(Vector3 pos) {
      camera.position = { (float)pos.x, (float)pos.y, (float)pos.z };
    }

    void setCameraTarget(Vector3 pos) {
      camera.target = { (float)pos.x, (float)pos.y, (float)pos.z };
    }

    void shutdown() {
      Raylib::CloseWindow();
      assert(false); // screw you warnings that won't go away
    }

    bool shouldClose() {
      return Raylib::WindowShouldClose();
    }

    float deltaTime() {
      return Raylib::GetFrameTime();
    }
};
