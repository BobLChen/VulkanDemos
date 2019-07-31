#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    float threshold;
    float padding0;
    float padding1;
    float padding2;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float threshold = filterParam.threshold;
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec4 textureColor = texture(inputImageTexture, textureCoordinate);
    float luminance = dot(textureColor.rgb, W);
    float thresholdResult = step(luminance, threshold);
    vec3 finalColor = abs(thresholdResult - textureColor.rgb);
    outFragColor = vec4(finalColor, textureColor.w);
}