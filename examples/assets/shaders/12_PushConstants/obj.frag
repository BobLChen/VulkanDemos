#version 450

layout (location = 0) in vec2 inUV0;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 normal = normalize(inNormal);
    float NDotL = clamp(dot(normal, vec3(0, 0, -1)), 0, 1.0);
    outFragColor = vec4(NDotL, NDotL, NDotL, 1.0);
}