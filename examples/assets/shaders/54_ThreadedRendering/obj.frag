#version 450

layout (location = 0) in vec2 inUV;

layout (binding  = 1) uniform sampler2D diffuseMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 diffuse = texture(diffuseMap, inUV);
    outFragColor = diffuse;
}