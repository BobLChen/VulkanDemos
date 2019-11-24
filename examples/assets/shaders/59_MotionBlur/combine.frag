#version 450

#define NUM_SAMPLES 15

layout (location = 0) in vec2 inUV0;

layout (binding = 1) uniform sampler2D originTexture;
layout (binding = 2) uniform sampler2D velocityTexture;

layout (binding = 3) uniform ParamBlock 
{
    vec4 data;
} paramData;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 pixelColor    = texture(originTexture, inUV0);
    vec4 pixelVelocity = texture(velocityTexture, inUV0);

    vec4 finalColor = vec4(0, 0, 0, 0);
    pixelVelocity.x = pixelVelocity.x;
    pixelVelocity.y = pixelVelocity.y * -1;
    pixelVelocity *= 2.0;
    pixelVelocity = min(pixelVelocity, 0.5);

    vec3 blurred = vec3(0, 0, 0);
    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        vec2 offset  = pixelVelocity.xy * i / NUM_SAMPLES;
        vec4 current = texture(originTexture, inUV0 + offset);
        blurred.xyz += current.xyz;
    }

    finalColor.xyz = blurred / NUM_SAMPLES;
    finalColor.w   = 1.0;
    
    outFragColor = finalColor;

    if (paramData.data.x < 1) {
        outFragColor = pixelColor;
    }
}