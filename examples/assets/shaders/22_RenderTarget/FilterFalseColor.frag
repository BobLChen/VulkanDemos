#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    vec4 firstColor;
    vec4 secondColor;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 firstColor  = filterParam.firstColor.xyz;
    vec3 secondColor = filterParam.secondColor.xyz;
    
    const vec3 luminanceWeighting = vec3(0.2125, 0.7154, 0.0721);
    vec4 textureColor = texture(inputImageTexture, textureCoordinate);
    float luminance = dot(textureColor.rgb, luminanceWeighting);
    outFragColor = vec4(mix(firstColor.rgb, secondColor.rgb, luminance), textureColor.a);
}