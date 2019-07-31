#version 450

layout (location = 0) in vec2 textureCoordinate;

layout (binding  = 1) uniform sampler2D inputImageTexture;

layout (binding = 0) uniform FilterParamBlock 
{
    float imageWidthFactor;
    float imageHeightFactor;
    float pixel;
    float padding;
} filterParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float pixel = filterParam.pixel;
    float imageWidthFactor  = filterParam.imageWidthFactor;
    float imageHeightFactor = filterParam.imageHeightFactor;

    vec2 uv  = textureCoordinate.xy;
    float dx = pixel * imageWidthFactor;
    float dy = pixel * imageHeightFactor;
    vec2 coord = vec2(dx * floor(uv.x / dx), dy * floor(uv.y / dy));
    vec3 tc = texture(inputImageTexture, coord).xyz;
    outFragColor = vec4(tc, 1.0);
}