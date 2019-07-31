#version 450

layout (location = 0) in vec2 textureCoordinate;
layout (location = 1) in vec2 leftTextureCoordinate;
layout (location = 2) in vec2 rightTextureCoordinate;

layout (location = 3) in vec2 topTextureCoordinate;
layout (location = 4) in vec2 topLeftTextureCoordinate;
layout (location = 5) in vec2 topRightTextureCoordinate;

layout (location = 6) in vec2 bottomTextureCoordinate;
layout (location = 7) in vec2 bottomLeftTextureCoordinate;
layout (location = 8) in vec2 bottomRightTextureCoordinate;

layout (binding = 0) uniform FilterParamBlock 
{
    float texelWidth;
    float texelHeight;
    vec2  padding0;
    
    mat4  convolutionMatrix;
} filterParam;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 bottomColor      = texture(inputImageTexture, bottomTextureCoordinate);
    vec4 bottomLeftColor  = texture(inputImageTexture, bottomLeftTextureCoordinate);
    vec4 bottomRightColor = texture(inputImageTexture, bottomRightTextureCoordinate);
    vec4 centerColor      = texture(inputImageTexture, textureCoordinate);
    vec4 leftColor        = texture(inputImageTexture, leftTextureCoordinate);
    vec4 rightColor       = texture(inputImageTexture, rightTextureCoordinate);
    vec4 topColor         = texture(inputImageTexture, topTextureCoordinate);
    vec4 topRightColor    = texture(inputImageTexture, topRightTextureCoordinate);
    vec4 topLeftColor     = texture(inputImageTexture, topLeftTextureCoordinate);

    mat4 convolutionMatrix = filterParam.convolutionMatrix;

    vec4 resultColor = vec4(0, 0, 0, 0);
    resultColor += topLeftColor    * convolutionMatrix[0][0] + topColor    * convolutionMatrix[0][1] + topRightColor    * convolutionMatrix[0][2];
    resultColor += leftColor       * convolutionMatrix[1][0] + centerColor * convolutionMatrix[1][1] + rightColor       * convolutionMatrix[1][2];
    resultColor += bottomLeftColor * convolutionMatrix[2][0] + bottomColor * convolutionMatrix[2][1] + bottomRightColor * convolutionMatrix[2][2];
    
    outFragColor = resultColor;
}