#pragma vertex
#version 330 core

uniform mat4 aVp;
layout (location = 0) in vec3 aVertex;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUv;
layout (location = 3) in mat4 aModel;

out vec3 FragPos;
out vec3 Normal;

void main() {

  vec4 worldPos = aModel * vec4(aVertex, 1.0);
  gl_Position = aVp * worldPos;
  FragPos = worldPos.xyz;

  Normal = transpose(inverse(mat3(aModel))) * aNormal;
}

#pragma fragment
#version 330 core

in vec3 Normal;
in vec3 FragPos;

out vec4 fragColor;

uniform float ambient;
uniform vec4 albedo;

vec3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir) {
  vec3 position = vec3(1000, 0, 0);
  float constant = 0.5;
  float linear = 0.5;
  float quadratic = 0.5;


  vec3 lightDir = normalize(position - fragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.5);
  float distance = length(position - fragPos);
  float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));

  return albedo.xyz * attenuation;
}

void main() {
  vec3  L       = normalize(vec3(0.6, 1.0, 0.4));
  float diffuse = max(dot(normalize(Normal), L), 0.0);
  float light   = ambient + (1 - ambient) * diffuse;
  //vec3 r = calcPointLight(Normal, FragPos, normalize(vec3(0, 0, 0) - FragPos));
  fragColor     = vec4(albedo.rgb * light, albedo.a);
}
