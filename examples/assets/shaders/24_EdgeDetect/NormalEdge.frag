#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D diffuseTexture;
layout (binding  = 2) uniform sampler2D normalsTexture;

layout (binding = 0) uniform FilterParam 
{
    vec4 size;
} param;

layout (location = 0) out vec4 outFragColor;

void main() 
{

    const vec2 PixelKernel[4] =
    {
        { 0,  1 },
        { 1,  0 },
        { 0, -1 },
        {-1,  0 }
    };
    
    vec4 oriNormal = texture(normalsTexture, inUV0);
    oriNormal = oriNormal * 2 - 1;

    vec4 sum = vec4(0);
    for (int i = 0; i < 4; ++i)
    {
        vec4 dstNormal = texture(normalsTexture, inUV0 + PixelKernel[i] / param.size.xy);
        dstNormal = dstNormal * 2 - 1;

        float odotd = dot(oriNormal.xyz, dstNormal.xyz);
        odotd = min(odotd, 1.0);
        odotd = max(odotd, 0.0);
        sum += 1.0 - odotd;
    }

    vec4 diffuse = texture(diffuseTexture, inUV0);
    if (param.size.z > 1.0) {
        outFragColor = sum * diffuse;
    } else {
        outFragColor = sum;
    }
}