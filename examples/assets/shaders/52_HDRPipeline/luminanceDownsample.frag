#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D originTexture;

layout (location = 0) out vec4 outFragColor;

vec2 PixelKernel[5] =
{
    {  1,  0 },
    { -1,  0 },
    {  0,  0 },
    {  0, -1 },
    {  0,  1 },
};

void main() 
{
    const vec3 weight = vec3(0.299, 0.587, 0.114);
    ivec2 texDim = textureSize(originTexture, 0);
    float average = 0;

    for (int i = 0; i < 5; ++i)
    {
        vec4 originColor = texture(originTexture, inUV0 + PixelKernel[i] / texDim);
        float luminance = dot(originColor.xyz, weight);
        average += log(0.00001 + luminance);
    }
    
    average *= 0.2;
    average  = exp(average);
    outFragColor = vec4(average, average, average, 1.0);
}