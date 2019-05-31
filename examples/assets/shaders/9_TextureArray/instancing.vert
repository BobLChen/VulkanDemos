#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;

#define NUM_INSTANCE 8

layout (binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
	mat4 models[NUM_INSTANCE];
	vec4 indexs[NUM_INSTANCE];
} ubo;

layout (location = 0) out vec3  outUV0;

void main() 
{
	outUV0      = vec3(inUV0, ubo.indexs[gl_InstanceIndex].x);
	gl_Position = ubo.projection * ubo.view * ubo.models[gl_InstanceIndex] * vec4(inPosition, 1.0);
}