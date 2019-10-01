#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform sampler2D originTexture;
layout (binding  = 2) uniform sampler2D ssaoTexture;

layout (binding = 3) uniform DebugParam 
{
    vec4 data;
} paramData;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 originColor = texture(originTexture, inUV0);
    float ssaoColor  = texture(ssaoTexture, inUV0).x;
    vec4 finalColor  = vec4(0, 0, 0, 0);

    if (paramData.data.x == 0) {
        finalColor = originColor * ssaoColor;
    }
    else if (paramData.data.x == 1) {
        finalColor = originColor;
    }
    else if (paramData.data.x == 2) {
        finalColor = vec4(ssaoColor);
    }
    else {
        finalColor = vec4(1, 1, 1, 1);
    }

    outFragColor = finalColor;
}