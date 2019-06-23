#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 normal = normalize(inNormal);
    float ndl   = dot(normal, vec3(0, 0, -1));
    outFragColor = inColor * ndl;
}
