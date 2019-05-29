#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;

struct Instance
{
	mat4 model;
	vec4 index;
};

layout (binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
	Instance instances[8];
} ubo;

layout (location = 0) out vec2  outUV0;
layout (location = 1) out float outIndex;

void main() 
{
	outUV0      = inUV0;
	outIndex    = ubo.instances[gl_InstanceIndex].index.x;
	gl_Position = ubo.projection * ubo.view * ubo.instances[gl_InstanceIndex].model * vec4(inPosition, 1.0);
}