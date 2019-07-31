#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    float fractionalWidthOfPixel;
    float aspectRatio;
    float padding0;
    float padding1;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 W = vec3(0.2125, 0.7154, 0.0721);
    
    float fractionalWidthOfPixel = filterParam.fractionalWidthOfPixel;
    float aspectRatio = filterParam.aspectRatio;

    vec2 sampleDivisor = vec2(fractionalWidthOfPixel, fractionalWidthOfPixel / aspectRatio);
    vec2 samplePos = textureCoordinate - mod(textureCoordinate, sampleDivisor) + 0.5 * sampleDivisor;
    vec2 textureCoordinateToUse = vec2(textureCoordinate.x, (textureCoordinate.y * aspectRatio + 0.5 - 0.5 * aspectRatio));
    vec2 adjustedSamplePos = vec2(samplePos.x, (samplePos.y * aspectRatio + 0.5 - 0.5 * aspectRatio));
    float distanceFromSamplePoint = distance(adjustedSamplePos, textureCoordinateToUse);
    vec3 sampledColor = texture(inputImageTexture, samplePos).rgb;
    float dotScaling = 1.0 - dot(sampledColor, W);
    float checkForPresenceWithinDot = 1.0 - step(distanceFromSamplePoint, (fractionalWidthOfPixel * 0.5) * dotScaling);

    outFragColor = vec4(vec3(checkForPresenceWithinDot), 1.0);
}