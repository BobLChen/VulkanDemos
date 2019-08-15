#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;

layout (binding  = 2) uniform sampler2D diffuseMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 diffuse  = texture(diffuseMap, inUV);
    vec3 lightDir = vec3(0, 1, -1);
    diffuse.xyz   = dot(lightDir, inNormal) * diffuse.xyz; 

    outFragColor  = diffuse;
}