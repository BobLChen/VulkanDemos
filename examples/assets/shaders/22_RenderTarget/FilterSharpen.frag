#version 450

layout (location = 0) in vec2 textureCoordinate;
layout (location = 1) in vec2 leftTextureCoordinate;
layout (location = 2) in vec2 rightTextureCoordinate;
layout (location = 3) in vec2 topTextureCoordinate;
layout (location = 4) in vec2 bottomTextureCoordinate;
layout (location = 5) in float centerMultiplier;
layout (location = 6) in float edgeMultiplier;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 textureColor = texture(inputImageTexture, textureCoordinate).rgb;
    vec3 leftTextureColor = texture(inputImageTexture, leftTextureCoordinate).rgb;
    vec3 rightTextureColor = texture(inputImageTexture, rightTextureCoordinate).rgb;
    vec3 topTextureColor = texture(inputImageTexture, topTextureCoordinate).rgb;
    vec3 bottomTextureColor = texture(inputImageTexture, bottomTextureCoordinate).rgb;

    outFragColor = vec4((textureColor * centerMultiplier - (leftTextureColor * edgeMultiplier + rightTextureColor * edgeMultiplier + topTextureColor * edgeMultiplier + bottomTextureColor * edgeMultiplier)), texture(inputImageTexture, bottomTextureCoordinate).w);
}