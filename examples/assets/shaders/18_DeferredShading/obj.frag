#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inPosition;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outNormal;

void main() 
{
    vec3 normal = normalize(inNormal);
    float NDotL = clamp(dot(normal, vec3(0, 0, -1)), 0, 1.0);

    outNormal    = vec4(inNormal, 1.0);
    outFragColor = vec4(NDotL, NDotL, NDotL, 1.0);
}