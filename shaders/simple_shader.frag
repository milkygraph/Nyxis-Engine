#version 450

layout (location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstant {
    mat2 transform;
    vec2 offset;
    vec3 color;
} pushConstant;

void main() {
    outColor = vec4(pushConstant.color, 1.0);
}