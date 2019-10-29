#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D originTexture;
layout (binding  = 2) uniform sampler2D volumeTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 originColor = texture(originTexture, inUV0);
    vec4 volumeColor = texture(volumeTexture, inUV0);
    vec4 finalColor  = originColor + volumeColor;
    finalColor = volumeColor;
    outFragColor = finalColor;
}