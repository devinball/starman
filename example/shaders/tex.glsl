#pragma vertex
#version 330 core

uniform mat4 aVp;
layout (location = 0) in vec3 aVertex;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;
layout (location = 3) in mat4 aModel;

out vec3 vPos;
out vec3 vNormal;
out vec2 vUv;

void main() {
  vec4 worldPos = aModel * vec4(aVertex, 1.0);
  gl_Position = aVp * worldPos;

  vNormal = transpose(inverse(mat3(aModel))) * aNormal;
  vUv = aUv;

  vPos = worldPos.xyz;
}

#pragma fragment
#version 330 core

in vec3 vPos;
in vec3 vNormal;
in vec2 vUv;

out vec4 fragColor;

uniform float ambient;
uniform float scale;
uniform sampler2D tex;

#define MAX_LIGHTS 64

struct PointLight {
  vec4 posIntensity;
  vec4 colorRange;
};

layout (std140) uniform Lights {
  PointLight lights[MAX_LIGHTS];
  int lightCount;
};

vec3 light(vec3 p, vec3 n) {
  vec3 lightAccum = vec3(ambient);
  for (int i = 0; i < lightCount; ++i) {
    vec3 toL = lights[i].posIntensity.xyz - vPos;
    float d = length(toL);
    vec3 L = toL / max(d, 0.0001);

    float atten = clamp(1.0 - d / lights[i].colorRange.w, 0.0, 1.0);
    atten *= atten;

    float diffuse = max(dot(n, L), 0.0);
    lightAccum += lights[i].colorRange.rgb * lights[i].posIntensity.a * diffuse * atten;
  }

  return lightAccum;
}

void main() {
  vec4 albedo = texture(tex, scale * vUv);
  vec3 l = light(vPos, vNormal);
  float test = 0.0;

  for (int i = 0; i < lightCount; ++i) {
    test += 0.4;
  }

  fragColor = vec4(albedo.rgb * test, 1.0); // vec4(albedo.rgb * ambient * 10, albedo.a); // * is component multiplication
}
