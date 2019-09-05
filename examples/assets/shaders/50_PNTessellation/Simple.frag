#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float diffuse = dot(inNormal, normalize(vec3(1, 0, -1)));
    outFragColor = vec4(diffuse, diffuse, diffuse, 1.0);
}