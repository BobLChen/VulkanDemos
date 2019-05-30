#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;

layout (binding = 0) uniform UBO 
{
	mat4 view;
	mat4 projection;
	mat4 models[8];
	float indexs[8];
} ubo;

layout (location = 0) out vec2  outUV0;
layout (location = 1) out float outIndex;

void main() 
{
	outUV0      = inUV0;
	outIndex    = ubo.indexs[gl_InstanceIndex];
	gl_Position = ubo.projection * ubo.view * ubo.models[gl_InstanceIndex] * vec4(inPosition, 1.0);
}