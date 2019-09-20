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
    const vec3 W = vec3(0.299, 0.587, 0.114);
    ivec2 texDim = textureSize(originTexture, 0);
    vec4 originColor = vec4(0, 0, 0, 0);

    for (int i = 0; i < 5; ++i)
    {
        originColor += texture(originTexture, inUV0 + PixelKernel[i] / texDim);
    }

    originColor *= 0.2;
    
    float bright = dot(originColor.xyz, W);
    if (bright >= 1.0)
    {
        outFragColor = vec4(originColor.xyz - 1.0, 1.0);
    }
    else
    {
        outFragColor = vec4(0, 0, 0, 1);
    }
}