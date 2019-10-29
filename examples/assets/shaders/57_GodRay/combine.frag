#version 450

layout (location = 0) in vec2 inUV0;

layout (binding = 1) uniform sampler2D originTexture;
layout (binding = 2) uniform sampler2D lightTexture;

layout (binding = 3) uniform DebugParam 
{
    vec4 param;
    vec4 color;
} paramData;

layout (location = 0) out vec4 outFragColor;

#define NUM_SAMPLES 30

void main() 
{
    float Density  = paramData.param.x;
    float Decay    = paramData.param.y;
    float Weight   = paramData.param.z;
    float Exposure = paramData.param.w;
    vec3 rayColor  = paramData.color.xyz;

    vec2 ScreenLightPos = vec2(0.5, 0.5);
    vec2 deltaTexCoord  = (inUV0 - ScreenLightPos.xy);
    deltaTexCoord *= 1.0 / NUM_SAMPLES * Density;

    vec2 texCoord = inUV0;
    vec3 retColor = vec3(0, 0, 0);
    
    float intensity = 1.0;
    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        texCoord -= deltaTexCoord;
        vec3 sampleColor = texture(lightTexture, texCoord).xyz;
        sampleColor *= intensity * Weight;
        intensity *= Decay;
        retColor += sampleColor;
    }

    retColor *= Exposure;
    retColor *= rayColor;

    vec4 background = texture(originTexture, inUV0);

    outFragColor.xyz = retColor + background.xyz;
    outFragColor.w   = 1.0;
}