#version 450

layout (set = 0, binding = 1) uniform sampler3D samplerColorMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inUV0;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec4 color = texture(samplerColorMap, inUV0);
	outFragColor = color;
}