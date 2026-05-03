#version 330 core

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 aVertex;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;
layout (location = 3) in mat4 aTransform;

out vec3 vNormal;

void main() {
  gl_Position = aTransform * vec4(aVertex, 1.0f);
  vNormal = normalize(mat3(aTransform) * aNormal);;
}

#endif
#ifdef FRAGMENT_SHADER

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

#endif