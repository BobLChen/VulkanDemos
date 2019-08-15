#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D sourceTexture;

layout (binding = 0) uniform FXAAParamBlock 
{
    vec4 frameRect;
} fxaaParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    outFragColor = texture(sourceTexture, inUV0);
}