#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;

layout (binding  = 1) uniform sampler2D diffuseMap;

layout (binding = 2) uniform ParamBlock 
{
    vec4 intensity;
} paramData;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 diffuse = texture(diffuseMap, inUV);
    diffuse.xyz *= paramData.intensity.x;
    outFragColor = diffuse;
}