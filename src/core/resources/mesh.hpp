#pragma once

#include "core/resources/resource.hpp"
#include "core/math/vector.hpp"

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <iostream>
#include <stdint.h>

struct Mesh : Resource {
  using Resource::Resource;

  std::vector<Vector3F> vertices;
  std::vector<Vector3F> normals;
  std::vector<Vector2F> uvs;
  std::vector<Vector4F> tangents;
  std::vector<uint32_t> indices;

  bool load() override {
    // can't load anything if the resource isn't valid
    if (!isValid()) { return false; }

    const struct aiScene* scene = aiImportFile(
      getId().c_str(),
      aiProcess_CalcTangentSpace |
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_SortByPType
    );

    if (scene == NULL) {
      //printf(aiGetErrorString());
      printf("something went wrong when loading a mesh");
      return false;
    }

    // process the scene

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
      const aiMesh* mesh = scene->mMeshes[m];
      const int indexOffset = static_cast<int>(vertices.size());

      for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
      {
        vertices.push_back({ mesh->mVertices[i].x,
                              mesh->mVertices[i].y,
                              mesh->mVertices[i].z });

        normals.push_back(mesh->HasNormals()
            ? Vector3F{ mesh->mNormals[i].x,
                        mesh->mNormals[i].y,
                        mesh->mNormals[i].z }
            : Vector3F{ 0.f, 1.f, 0.f });

        uvs.push_back(mesh->HasTextureCoords(0)
            ? Vector2F{ mesh->mTextureCoords[0][i].x,
                        mesh->mTextureCoords[0][i].y }
            : Vector2F{ 0.f, 0.f });
      }

      for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
      {
          const aiFace& face = mesh->mFaces[f];
          for (unsigned int j = 0; j < face.mNumIndices; ++j)
              indices.push_back(indexOffset + static_cast<int>(face.mIndices[j]));
      }
    }
    
    std::cout << "Loaded mesh: " << getId() << std::endl;

    aiReleaseImport(scene);

    return true;
  }

  bool unload() override {
    vertices.clear();
    normals.clear();
    uvs.clear();
    indices.clear();
    tangents.clear();

    return false;
  }
};

/*
Mesh createCubeMesh() {
  Mesh m;
  return m;
}

Mesh createSphereMesh() {
  Mesh m;
  return m;
}
*/
