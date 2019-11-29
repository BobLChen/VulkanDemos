#version 450

#define NUM_SAMPLES 15

layout (location = 0) in vec2 inUV0;

layout (binding = 1) uniform sampler2D originTexture;
layout (binding = 2) uniform sampler2D peel0Texture;
layout (binding = 3) uniform sampler2D peel1Texture;
layout (binding = 4) uniform sampler2D peel2Texture;
layout (binding = 5) uniform sampler2D peel3Texture;
layout (binding = 6) uniform sampler2D peel4Texture;

layout (binding = 7) uniform ParamBlock 
{
    vec4 layer;
} uboParam;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 pixelColor = texture(originTexture, inUV0);

    vec4 peel0Color = texture(peel0Texture, inUV0);
    vec4 peel1Color = texture(peel1Texture, inUV0);
    vec4 peel2Color = texture(peel2Texture, inUV0);
    vec4 peel3Color = texture(peel3Texture, inUV0);
    vec4 peel4Color = texture(peel4Texture, inUV0);

    vec4 finalColor;

    if (uboParam.layer.z < 1) {
        vec4 dstColor;
        vec4 srcColor;
        // peel4
        dstColor = peel4Color;
        // peel3
        srcColor = peel3Color;
        dstColor = vec4(mix(dstColor.rgb, srcColor.rgb, srcColor.a), srcColor.a);
        // peel2
        srcColor = peel2Color;
        dstColor = vec4(mix(dstColor.rgb, srcColor.rgb, srcColor.a), srcColor.a);
        // peel1
        srcColor = peel1Color;
        dstColor = vec4(mix(dstColor.rgb, srcColor.rgb, srcColor.a), srcColor.a);
        // peel0
        srcColor = peel0Color;
        dstColor = vec4(mix(dstColor.rgb, srcColor.rgb, srcColor.a), srcColor.a);
        // background
        srcColor = dstColor;
        dstColor = pixelColor;
        dstColor = vec4(mix(dstColor.rgb, srcColor.rgb, srcColor.a), 1.0);

        finalColor = dstColor;
    }
    else if (uboParam.layer.z < 2) {
        finalColor = pixelColor;
    }
    else if (uboParam.layer.z < 3) {
        finalColor = peel0Color;
    }
    else if (uboParam.layer.z < 4) {
        finalColor = peel1Color;
    }
    else if (uboParam.layer.z < 5) {
        finalColor = peel2Color;
    }
    else if (uboParam.layer.z < 6) {
        finalColor = peel3Color;
    }
    else if (uboParam.layer.z < 7) {
        finalColor = peel4Color;
    }

    outFragColor = finalColor;
}