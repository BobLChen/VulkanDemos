#version 450


layout (location = 0) in vec2 inUV0;

layout (binding = 1) uniform sampler2D diffuseMap;
layout (binding = 2) uniform sampler3D lutMap;

layout (binding = 3) uniform LutDebugBlock
{
	float   bias;
    vec3    padding;
} uboLutDebug;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 color = texture(diffuseMap, inUV0);
    outFragColor = texture(lutMap, color.xyz);
}