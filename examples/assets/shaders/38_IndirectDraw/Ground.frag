#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec4 inLightViewPos;

layout (binding = 1) uniform LightsMVPBlock 
{
	mat4 viewMatrix;
	vec4 cascadeScale[4];
	vec4 cascadeOffset[4];
	mat4 projMatrix[4];
	vec4 offset[4];
	vec4 direction;
	vec4 bias;
    vec4 debug;
} lightMVP;

layout (binding = 2) uniform sampler2D shadowMap;

layout (location = 0) out vec4 outFragColor;

const vec2 Poisson25[25] = vec2[](
    vec2(-0.978698, -0.0884121),
    vec2(-0.841121, 0.521165),
    vec2(-0.71746, -0.50322),
    vec2(-0.702933, 0.903134),
    vec2(-0.663198, 0.15482),
    vec2(-0.495102, -0.232887),
    vec2(-0.364238, -0.961791),
    vec2(-0.345866, -0.564379),
    vec2(-0.325663, 0.64037),
    vec2(-0.182714, 0.321329),
    vec2(-0.142613, -0.0227363),
    vec2(-0.0564287, -0.36729),
    vec2(-0.0185858, 0.918882),
    vec2(0.0381787, -0.728996),
    vec2(0.16599, 0.093112),
    vec2(0.253639, 0.719535),
    vec2(0.369549, -0.655019),
    vec2(0.423627, 0.429975),
    vec2(0.530747, -0.364971),
    vec2(0.566027, -0.940489),
    vec2(0.639332, 0.0284127),
    vec2(0.652089, 0.669668),
    vec2(0.773797, 0.345012),
    vec2(0.968871, 0.840449),
    vec2(0.991882, -0.657338)
);

const vec4 cascadeColor[4] = 
{
    vec4 (1.0f, 0.0f, 0.0f, 1.0f),
    vec4 (0.0f, 1.0f, 0.0f, 1.0f),
    vec4 (0.0f, 0.0f, 1.0f, 1.0f),
    vec4 (1.0f, 0.0f, 1.0f, 1.0f),
};

void main() 
{
    ivec2 texDim     = textureSize(shadowMap, 0);
    int cascadeFound = 0;
    int cascadeIndex = 0;
    vec4 shadowMapTextureCoord = vec4(0, 0, 0, 0);

    float maxBorder = (texDim.x - 1.0f) / texDim.x;
    float minBorder = 1.0f / texDim.x;

    for (int index = 0; index < 4 && cascadeFound == 0; ++index)
    {
        shadowMapTextureCoord  = inLightViewPos * lightMVP.cascadeScale[index];
        shadowMapTextureCoord += lightMVP.cascadeOffset[index];
        if (min(shadowMapTextureCoord.x, shadowMapTextureCoord.y) > minBorder && max(shadowMapTextureCoord.x, shadowMapTextureCoord.y) < maxBorder)
        {
            cascadeIndex = index;
            cascadeFound = 1;
        }
    }

    vec4 lightProjPos  = lightMVP.projMatrix[cascadeIndex] * inLightViewPos;
    lightProjPos.xyzw /= lightProjPos.w;
	lightProjPos.xy    = lightProjPos.xy * 0.5 + 0.5;
	lightProjPos.y     = 1.0 - lightProjPos.y;
    
    vec3 shadowCoord = vec3(lightProjPos.xy * lightMVP.bias.zw + lightMVP.offset[cascadeIndex].xy, lightProjPos.z);

    vec4 diffuse  = vec4(inColor, 1.0);
    vec3 lightDir = normalize(lightMVP.direction.xyz);
    
    diffuse.xyz   = dot(lightDir, inNormal) * diffuse.xyz; 

    float depth0  = shadowCoord.z - lightMVP.bias.x;
    vec2 texStep  = vec2(1.0 / texDim.x / 2, 1.0 / texDim.y / 2);

    float shadow = 0.0;
    for (int i = 0; i < 25; ++i)
    {
        vec2 offset  = Poisson25[i] * texStep * lightMVP.bias.y;
        float depth1 = texture(shadowMap, shadowCoord.xy + offset).r;
        shadow += depth0 >= depth1 ? 0.0 : 1.0;
    }
    shadow /= 25;

    diffuse.xyz *= shadow;

    if (lightMVP.debug.x > 0) {
        diffuse.xyz *= cascadeColor[cascadeIndex].xyz;
    }

    outFragColor = diffuse;
}