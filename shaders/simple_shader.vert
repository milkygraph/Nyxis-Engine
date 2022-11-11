#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform Push {
  mat4 transform; // projection * view * model
  mat4 modelMatrix; 
} push;

const vec3 DIRECTIONAL_LIGHT_DIRECTION = normalize(vec3(1.0, 1.0, 1.0));
const float AMBIENT = 0.02;

void main() {
    gl_Position = push.transform * vec4(position, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(push.modelMatrix)));
    vec3 normalWorldSpace = normalize(normalMatrix * normal);
    

    float light = AMBIENT + max(dot(normalWorldSpace, DIRECTIONAL_LIGHT_DIRECTION), 0);

    fragColor = light * color;
}