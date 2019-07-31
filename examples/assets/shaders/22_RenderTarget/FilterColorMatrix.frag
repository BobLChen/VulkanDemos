#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    mat4 colorMatrix;

    float intensity;
    float padding0;
    float padding1;
    float padding2;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    mat4 colorMatrix  = filterParam.colorMatrix;
    float intensity   = filterParam.intensity;

    vec4 textureColor = texture(inputImageTexture, textureCoordinate);
    vec4 outputColor  = textureColor * colorMatrix;
    outFragColor      = (intensity * outputColor) + ((1.0 - intensity) * textureColor);
}