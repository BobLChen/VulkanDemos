#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    float colorLevels;
    float padding0;
    float padding1;
    float padding2;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float colorLevels = filterParam.colorLevels;

    vec4 textureColor = texture(inputImageTexture, textureCoordinate);
    outFragColor = floor((textureColor * colorLevels) + vec4(0.5)) / colorLevels;
}