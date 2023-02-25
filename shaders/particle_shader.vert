#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

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

layout(location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragOffset;

void main()
{
	// instanced drawing 
	uint index = gl_InstanceIndex;
	Particle particle = particleSet.particles[index];

	fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
	vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

	vec3 positionWorld = particle.position.xyz
		+ particle.size.x * fragOffset.x * cameraRightWorld
		+ particle.size.x * fragOffset.y * cameraUpWorld;

	fragColor = particle.color.xyz;
	gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);
}