#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBiTangent;

layout (binding = 2) uniform sampler2D   texAlbedo;
layout (binding = 3) uniform sampler2D   texNormal;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    mat3 TBN = mat3(inTangent, inBiTangent, inNormal);

    vec4 texAlbedoColor   = texture(texAlbedo, inUV);
    vec4 texNormalColor   = texture(texNormal, inUV);

    vec3 albedo = texAlbedoColor.xyz;
    vec3 normal = TBN * normalize(texNormalColor.xyz * 2.0 - 1.0);
    
    outFragColor.xyz = albedo.xyz;
    outFragColor.w   = 1.0;
}