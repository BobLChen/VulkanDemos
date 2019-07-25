#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inPosition;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outPosition;

void main() 
{
    vec3 normal = normalize(inNormal);
    outNormal    = vec4(inNormal, 1.0);
    outFragColor = vec4(1.0, 1.0, 1.0, 1.0);
    outPosition  = vec4(inPosition, 1.0);
}