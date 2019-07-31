#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    vec4 threshold;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float threshold = filterParam.threshold.x;
    const vec3 W = vec3(0.2125, 0.7154, 0.0721);
    vec4 textureColor = texture(inputImageTexture, textureCoordinate);
    float luminance = dot(textureColor.rgb, W);
    float thresholdResult = step(threshold, luminance);
    outFragColor = vec4(vec3(thresholdResult), textureColor.w);
}