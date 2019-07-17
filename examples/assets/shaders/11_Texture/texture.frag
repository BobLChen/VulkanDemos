#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;

layout (binding = 2) uniform sampler2D diffuseMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 normal     = normalize(inNormal);
    vec3 lightDir   = vec3(0, 0, -1);
    float ndl       = dot(normal, lightDir);
    vec3 lightColor = vec3(1.0, 1.0, 1.0) * ndl;
    vec4 diffuse    = texture(diffuseMap, inUV);
    diffuse.xyz    *= lightColor.xyz;
    outFragColor    = diffuse;
}