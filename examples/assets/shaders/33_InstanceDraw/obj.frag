#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D diffuseMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 normal = normalize(inNormal);
    vec3 light  = vec3(0, 1, -1);
    float NDotL = dot(normal, normalize(light));
    vec4 color  = texture(diffuseMap, inUV0);
    outFragColor = vec4(color.xyz * NDotL + 0.15, 1.0);
}