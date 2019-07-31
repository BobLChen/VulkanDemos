#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    vec4 filterColorIntensity;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 filterColor = filterParam.filterColorIntensity.xyz;
    float intensity = filterParam.filterColorIntensity.w;
    const vec3 luminanceWeighting = vec3(0.2125, 0.7154, 0.0721);
    vec4 textureColor = texture(inputImageTexture, textureCoordinate);
    float luminance = dot(textureColor.rgb, luminanceWeighting);
    vec4 desat = vec4(vec3(luminance), 1.0);
    vec4 outputColor = vec4(
        (desat.r < 0.5 ? (2.0 * desat.r * filterColor.r) : (1.0 - 2.0 * (1.0 - desat.r) * (1.0 - filterColor.r))),
        (desat.g < 0.5 ? (2.0 * desat.g * filterColor.g) : (1.0 - 2.0 * (1.0 - desat.g) * (1.0 - filterColor.g))),
        (desat.b < 0.5 ? (2.0 * desat.b * filterColor.b) : (1.0 - 2.0 * (1.0 - desat.b) * (1.0 - filterColor.b))),
        1.0
    );
    outFragColor = vec4( mix(textureColor.rgb, outputColor.rgb, intensity), textureColor.a);
}