#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in float inLayer;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float NDotL = clamp(dot(inNormal, vec3(0, 1, 0)), 0, 1.0);
    outFragColor = vec4(inColor, 1.0) * (NDotL + 0.25);
}