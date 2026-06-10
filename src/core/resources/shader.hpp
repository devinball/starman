#pragma once

#include "core/resources/resource.hpp"

#include <string>
#include <fstream>

inline constexpr const char* defaultVertexShader = R"glsl(
#version 330 core

uniform mat4 aVp;
layout (location = 0) in vec3 aVertex;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;
layout (location = 3) in mat4 aModel;

out vec3 vWorldPos;
out vec3 vNormal;

void main() {

  vec4 worldPos = aModel * vec4(aVertex, 1.0);
  gl_Position = aVp * worldPos;
  vWorldPos = worldPos.xyz;

  vNormal = transpose(inverse(mat3(aModel))) * aNormal; //normalize(mat3(transpose(inverse(aModel))) * aNormal);
}
)glsl";

inline constexpr const char* defaultFragmentShader = R"glsl(
#version 330 core
in vec3 vNormal;

out vec4 fragColor;

uniform float ambient;
uniform vec4 albedo;

void main() {
  vec4  base    = albedo;

  vec3  L       = normalize(vec3(0.6, 1.0, 0.4));
  float diffuse = max(dot(normalize(vNormal), L), 0.0);
  float light   = ambient + (1 - ambient) * diffuse;
  fragColor     = vec4(base.rgb * light, base.a);

  // fragColor = vec4(vNormal.rgb, 1.0);
}
)glsl";


// shader does not contain any renderer specific data
// the renderer needs to keep track of it
struct Shader : Resource {
  using Resource::Resource;
  
  std::string vertexSource;
  std::string fragmentSource;
  std::string geometrySource;

  uint shaderProgram;
  // eitehr this or a std::unordered_map<std::string, uint>
  // the map would hold id, shaderProgram
  // then you would reference a shader by id, the renderer
  // does all the translating

  // Kinda LLM-WRITTEN
  bool load(const std::string &path) {
    std::ifstream file(path.c_str());

    if (!file.is_open()) {
      printf("Failed to open shader: %s", path.c_str());
    }

    enum class Section { None, Vertex, Fragment, Geometry };

    std::unordered_map<Section, std::string*> targets = {
      { Section::Vertex, &vertexSource },
      { Section::Fragment, &fragmentSource },
      { Section::Geometry, &geometrySource },
    };

    std::unordered_map<std::string, Section> pragmas = {
      { "#pragma vertex", Section::Vertex },
      { "#pragma fragment", Section::Fragment },
      { "#pragma geometry", Section::Geometry },
    };

    Section current = Section::None;
    std::string line;

    while (std::getline(file, line)) {
      auto trimmed = line.substr(0, line.find_last_not_of(" \t\r\n") + 1);

      auto it = pragmas.find(trimmed);
      if (it != pragmas.end()) {
        current = it->second;
        continue;
      }

      if (current != Section::None) {
        *targets[current] += line + '\n';
      }
    }

    if (vertexSource.empty() || fragmentSource.empty()) {
      printf("Vertex and Fragment are needed for shader: %s", path.c_str());
      return false;
    }

    printf("Loaded shader: %s\n", path.c_str());

    return true;
  }

  bool unload() {
    return true;
  }
};
