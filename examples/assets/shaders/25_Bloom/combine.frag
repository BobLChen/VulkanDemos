#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D originTexture;
layout (binding  = 2) uniform sampler2D filterTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 originColor = texture(originTexture, inUV0);
    vec4 filterColor = texture(filterTexture, inUV0) * 10;
    outFragColor = originColor + filterColor;
}