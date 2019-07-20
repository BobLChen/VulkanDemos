#version 450

layout (location = 0) in vec2 inUV0;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBiTangent;
layout (location = 4) in vec4 inCustom;

layout (binding = 2) uniform sampler2DArray diffuseMap;

layout (binding = 3) uniform ParamBlock
{
    float step;
    float debug;
    float padding0;
    float padding1;
} params;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    // make tbn
    mat3 TBN = mat3(inTangent, inBiTangent, inNormal);

    vec4 normal0 = texture(diffuseMap, vec3(inUV0, inCustom.x + params.step * 1)) * inCustom.y;
    vec4 normal1 = texture(diffuseMap, vec3(inUV0, inCustom.z + params.step * 1)) * inCustom.w;
    vec3 normal  = normal0.xyz + normal1.xyz;
    normal = normalize(2.0 * normal - vec3(1.0, 1.0, 1.0));
    normal = TBN * normal;

    vec4 finalColor = vec4(0, 0, 0, 0);

    if (params.debug <= 0)
    {
        vec4 albedo0 = texture(diffuseMap, vec3(inUV0, inCustom.x)) * inCustom.y;
        vec4 albedo1 = texture(diffuseMap, vec3(inUV0, inCustom.z)) * inCustom.w;
        finalColor = albedo0 + albedo1;
    }
    else
    {
        vec4 albedo0 = texture(diffuseMap, vec3(inUV0, inCustom.x + params.step * 2)) * inCustom.y;
        vec4 albedo1 = texture(diffuseMap, vec3(inUV0, inCustom.z + params.step * 2)) * inCustom.w;
        finalColor = albedo0 + albedo1;
    }
    
    float NDotL = clamp(dot(normal, vec3(0, 1, 0)), 0, 1.0);
    finalColor = finalColor * NDotL;
    outFragColor = finalColor;
}