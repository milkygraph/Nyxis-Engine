#version 450

layout(location = 0) in vec3 fragColor;
layout (location = 1) in vec2 fragOffset;

layout (location = 0) out vec4 outColor;

struct PointLight
{
	vec4 position; // ignore w
	vec4 color;
	vec3 velocity;
	vec2 size;
};

struct Particle
{
	vec4 position; // ignore w
	vec4 color;
	vec3 velocity;
	vec2 size;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  mat4 invView;
  vec4 ambientLightColor; // w is intensity
  PointLight pointLights[10];
  int numLights;
} ubo;

layout (set = 1, binding = 0) buffer Particles
{  
	Particle particles[1000];
} particleSet;

void main()
{
	float dis = sqrt(dot(fragOffset, fragOffset));
	if (dis >= 1.0) {
		discard;
	}
	outColor = vec4(fragColor.xyz, 1.0);
}