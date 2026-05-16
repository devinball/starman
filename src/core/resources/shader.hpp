#pragma once

#include "core/resources/resource.hpp"

#include <string>


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

void main() {
  float ambient = 0.1;
  vec3  L       = normalize(vec3(0.6, 1.0, 0.4));
  float diffuse = max(dot(normalize(vNormal), L), 0.0);
  float light   = ambient + (1 - ambient) * diffuse;
  vec4  base    = vec4(1.0, 1.0, 1.0, 1.0);
  fragColor     = vec4(base.rgb * light, base.a);

  // fragColor = vec4(vNormal.rgb, 1.0);
}
)glsl";

inline constexpr const char* defaultShader = R"glsl(

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

  bool load() override {
    vertexSource = defaultVertexShader;
    fragmentSource = defaultFragmentShader;

    return true;
  }

  bool unload() override {
    return true;
  }
};
