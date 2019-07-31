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

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float bottomLeftIntensity = texture(inputImageTexture, bottomLeftTextureCoordinate).r;
    float topRightIntensity = texture(inputImageTexture, topRightTextureCoordinate).r;
    float topLeftIntensity = texture(inputImageTexture, topLeftTextureCoordinate).r;
    float bottomRightIntensity = texture(inputImageTexture, bottomRightTextureCoordinate).r;
    float leftIntensity = texture(inputImageTexture, leftTextureCoordinate).r;
    float rightIntensity = texture(inputImageTexture, rightTextureCoordinate).r;
    float bottomIntensity = texture(inputImageTexture, bottomTextureCoordinate).r;
    float topIntensity = texture(inputImageTexture, topTextureCoordinate).r;
    
    vec2 gradientDirection;
    gradientDirection.x = -bottomLeftIntensity - 2.0 * leftIntensity - topLeftIntensity + bottomRightIntensity + 2.0 * rightIntensity + topRightIntensity;
    gradientDirection.y = -topLeftIntensity - 2.0 * topIntensity - topRightIntensity + bottomLeftIntensity + 2.0 * bottomIntensity + bottomRightIntensity;
    
    float gradientMagnitude = length(gradientDirection);
    vec2 normalizedDirection = normalize(gradientDirection);
    normalizedDirection = sign(normalizedDirection) * floor(abs(normalizedDirection) + 0.617316);
    normalizedDirection = (normalizedDirection + 1.0) * 0.5;
    
    outFragColor = vec4(gradientMagnitude, normalizedDirection.x, normalizedDirection.y, 1.0);
}