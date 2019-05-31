#version 450

layout (set = 0, binding = 1) uniform sampler2DArray samplerArray;

layout (location = 0) in vec3 inUV0;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = texture(samplerArray, inUV0);
}