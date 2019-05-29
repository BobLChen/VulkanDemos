#version 450

layout (set = 0, binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec2  inUV0;
layout (location = 1) in float inIdex;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = texture(samplerColorMap, inUV0);
}