#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    outFragColor = vec4(1, 0, 0, 1);
}