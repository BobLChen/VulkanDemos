#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    outFragColor = texture(inputImageTexture, inUV0);
}