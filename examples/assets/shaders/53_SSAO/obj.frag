#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in float inDepth;

layout (binding  = 1) uniform sampler2D diffuseMap;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out float outDepth;

void main() 
{
    vec4 diffuse = texture(diffuseMap, inUV);
    outFragColor = diffuse;
    outDepth     = inDepth;
}