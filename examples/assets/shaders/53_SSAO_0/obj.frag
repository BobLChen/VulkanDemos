#version 450

layout (location = 0) in vec3 inNormal;

layout (binding  = 1) uniform sampler2D diffuseMap;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outFragNormal;

void main() 
{
    float NDotL   = max(dot(inNormal, normalize(vec3(0, 1, 0))), 0);
    // outFragColor  = vec4(NDotL, NDotL, NDotL, 1.0);
    outFragColor = vec4(1, 1, 1, 1);
    
    vec3 normal   = normalize(inNormal);
    outFragNormal = vec4(normal * 0.5 + 0.5, 1.0);
}