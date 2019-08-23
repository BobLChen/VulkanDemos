#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec4 inLightViewPos;

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

layout (binding  = 2) uniform sampler2D shadowMap;

layout (location = 0) out vec4 outFragColor;

const vec4 cascadeColor[4] = 
{
    vec4(1.0f, 0.0f, 0.0f, 1.0f),
    vec4(0.0f, 1.0f, 0.0f, 1.0f),
    vec4(0.0f, 0.0f, 1.0f, 1.0f),
    vec4(1.0f, 0.0f, 1.0f, 1.0f),
};

void main() 
{
    int cascadeFound = 0;
    int cascadeIndex = 0;
    vec4 shadowMapTextureCoord = vec4(0, 0, 0, 0);
    for (int index = 0; index < 4 && cascadeFound == 0; ++index)
    {
        shadowMapTextureCoord  = inLightViewPos * lightMVP.cascadeScale[index];
        shadowMapTextureCoord += lightMVP.cascadeOffset[index];
        if (min(shadowMapTextureCoord.x, shadowMapTextureCoord.y) > 0.0 && max(shadowMapTextureCoord.x, shadowMapTextureCoord.y) < 1.0)
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

    vec4 diffuse  = vec4(1.0, 1.0, 1.0, 1.0);
    vec3 lightDir = normalize(lightMVP.direction.xyz);
    
    diffuse.xyz   = dot(lightDir, inNormal) * diffuse.xyz; 

    float depth0  = shadowCoord.z - lightMVP.bias.x;
    float depth1  = texture(shadowMap, shadowCoord.xy).r;
    float shadow  = 1.0;

    if (depth0 >= depth1) {
        shadow = 0.0;
    }

    diffuse.xyz *= shadow;

    if (lightMVP.debug.x > 0) {
        diffuse.xyz *= cascadeColor[cascadeIndex].xyz;
    }
    
    outFragColor = diffuse;
}