#version 450

const int KernelSize = 13;

const float BlurWeights[KernelSize] = 
{
    0.002216,
    0.008764,
    0.026995,
    0.064759,
    0.120985,
    0.176033,
    0.199471,
    0.176033,
    0.120985,
    0.064759,
    0.026995,
    0.008764,
    0.002216,
};

vec2 PixelKernel[KernelSize] =
{
    { -6, 0 },
    { -5, 0 },
    { -4, 0 },
    { -3, 0 },
    { -2, 0 },
    { -1, 0 },
    {  0, 0 },
    {  1, 0 },
    {  2, 0 },
    {  3, 0 },
    {  4, 0 },
    {  5, 0 },
    {  6, 0 },
};

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D diffuseTexture;

layout (binding  = 0) uniform FilterParam 
{
    vec4 size;
} param;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 finalColor = vec4(0);
    for (int i = 0; i < KernelSize; ++i)
    {
        vec2 uv = inUV0.xy + PixelKernel[i].xy / param.size.xy * param.size.z;
        vec4 color = texture(diffuseTexture, uv);
        finalColor += color * BlurWeights[i];
    }

    outFragColor = finalColor;
}