#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D originTexture;
layout (binding  = 2) uniform sampler2D bloomTexture;
layout (binding  = 3) uniform sampler2D luminanceTexture;

layout (binding = 4) uniform ParamBlock 
{
    vec4 intensity;
} param;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 originColor = texture(originTexture, inUV0);
    vec4 bloomColor  = texture(bloomTexture, inUV0);
    vec4 luminance  = texture(luminanceTexture, vec2(0.5, 0.5));
    
    vec4 finalColor  = originColor + bloomColor;

    float lp = (param.intensity.y / luminance.x) * max(finalColor.r, max(finalColor.g, finalColor.b));
    float lmsqr = (luminance.y + param.intensity.z * luminance.y) * (luminance.y + param.intensity.z * luminance.y);
    float toneScalar = (lp * (1.0 + (lp / lmsqr))) / (1.0 + lp);
    
    outFragColor = finalColor * toneScalar;
}