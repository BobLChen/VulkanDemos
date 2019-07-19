#version 450

layout (location = 0) in vec2 inUV0;
layout (location = 1) in vec3 inNormal;

layout (binding = 2) uniform sampler2DArray diffuseMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 normal = normalize(inNormal);
    float NDotL = clamp(dot(normal, vec3(0, 0, -1)), 0, 1.0);
    vec4 color  = texture(diffuseMap, vec3(inUV0, 1)) * NDotL;
    outFragColor = color;
}