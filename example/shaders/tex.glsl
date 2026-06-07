#pragma vertex
#version 330 core

uniform mat4 aVp;
layout (location = 0) in vec3 aVertex;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;
layout (location = 3) in mat4 aModel;

out vec3 vNormal;
out vec2 vUv;

void main() {

  vec4 worldPos = aModel * vec4(aVertex, 1.0);
  gl_Position = aVp * worldPos;

  vNormal = transpose(inverse(mat3(aModel))) * aNormal;
  vUv = aUv;
}

#pragma fragment
#version 330 core

in vec3 vNormal;
in vec2 vUv;

out vec4 fragColor;

uniform float ambient;
uniform sampler2D tex;

void main() {
  vec4 albedo = texture(tex, 10 * vUv);
  vec3  L       = normalize(vec3(0.6, 1.0, 0.4));
  float diffuse = max(dot(normalize(vNormal), L), 0.0);
  float light   = ambient + (1 - ambient) * diffuse;
  fragColor     = vec4(albedo.rgb * light, albedo.a);
}
