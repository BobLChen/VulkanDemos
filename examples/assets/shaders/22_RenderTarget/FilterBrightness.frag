#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding = 0) uniform FilterParamBlock 
{
    vec4 brightness;
} filterParam;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 textureColor = texture(inputImageTexture, textureCoordinate);
    outFragColor = vec4((textureColor.rgb + vec3(filterParam.brightness.x)), textureColor.w);
}