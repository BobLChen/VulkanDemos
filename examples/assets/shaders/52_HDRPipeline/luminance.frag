#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D originTexture;

layout (location = 0) out vec4 outFragColor;

vec2 PixelKernel[9] =
{
    { -1,  1 },
    {  0,  1 },
    {  1,  1 },
    {  1,  0 },
    {  1, -1 },
    {  0, -1 },
    { -1, -1 },
    { -1,  0 },
    {  0,  0 }
};

void main() 
{
    ivec2 texDim  = textureSize(originTexture, 0);
    float average = 0;

    for (int i = 0; i < 9; ++i)
    {
        vec4 color = texture(originTexture, inUV0 + PixelKernel[i] / texDim);
        average += color.x;
    }
    
    average /= 9.0;
    outFragColor = vec4(average, average, average, 1.0);
}