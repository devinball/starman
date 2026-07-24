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

  vNormal = normalize(transpose(inverse(mat3(aModel))) * aNormal);
  vUv = aUv;

  vPos = worldPos.xyz;

  float C = 1.f;
  float far = 100000000000000000.f;

  gl_Position.z = (2.0 * log(gl_Position.w * C + 1.0) / log(far * C + 1.0) - 1.0) * gl_Position.w;
}

#pragma fragment
#version 330 core

in vec3 vPos;
in vec3 vNormal;
in vec2 vUv;

out vec4 fragColor;

//uniform float specularStrength;
uniform float ambient;
uniform float scale;
uniform sampler2D tex;

#define MAX_LIGHTS 16

struct PointLight {
  vec3 position;
  vec3 color;
};

uniform PointLight pointLights[MAX_LIGHTS];
uniform int lightCount;

vec3 calcPointLight(PointLight light) {
  float specularStrength = 0.1f;

  vec3 lightDir = normalize(light.position - vPos);
  vec3 viewDir = normalize(-vPos);
  vec3 reflectDir = reflect(-lightDir, vNormal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = specularStrength * spec * light.color;

  float diffuse = max(dot(vNormal, lightDir), 0.0);
  return (vec3(ambient + diffuse) + specular) * light.color;
}

void main() {
  // !! camera is 0, 0, 0, which explains this issue, the shadows are just always away from the camera
  vec4 albedo = texture(tex, scale * vUv);
  vec3 l;
  for (int i = 0; i < lightCount; ++i) {
    l += calcPointLight(pointLights[i]);
  }
  fragColor = vec4(albedo.rgb * l, 1.0f); // vec4(normalize(pointLights[0].position - vPos), 1.0f); // vec4(albedo.rgb * l, 1.0);
}
