#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    vec4 levelMinimum;
    vec4 levelMiddle;
    vec4 levelMaximum;
    vec4 minOutput;
    vec4 maxOutput;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 levelMinimum = filterParam.levelMinimum.xyz;
    vec3 levelMiddle = filterParam.levelMiddle.xyz;
    vec3 levelMaximum = filterParam.levelMaximum.xyz;
    vec3 minOutput = filterParam.minOutput.xyz;
    vec3 maxOutput = filterParam.maxOutput.xyz;

    vec4 textureColor = texture(inputImageTexture, textureCoordinate);
    outFragColor = vec4(mix(minOutput, maxOutput, pow(min(max(textureColor.rgb -levelMinimum, vec3(0.0)) / (levelMaximum - levelMinimum), vec3(1.0)), 1.0 /levelMiddle)) , textureColor.a);
}