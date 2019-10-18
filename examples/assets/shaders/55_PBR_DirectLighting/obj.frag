#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBiTangent;

layout (binding = 1) uniform PBRParamBlock 
{
	vec4 param;
} uboParam;

layout (binding  = 2) uniform sampler2D texAlbedo;
layout (binding  = 3) uniform sampler2D texNormal;
layout (binding  = 4) uniform sampler2D texORMParam;

layout (location = 0) out vec4 outFragColor;

vec3 GammaToLinearSpace(vec3 sRGB)
{
    // Approximate version from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
}

float LinearToGammaSpaceExact(float value)
{
    if (value <= 0.0) {
        return 0.0;
    }
    else if (value <= 0.0031308) {
        return 12.92 * value;
    }
    else if (value < 1.0) {
        return 1.055 * pow(value, 0.4166667) - 0.055;
    }
    else {
        return pow(value, 0.45454545);
    }
}

vec3 LinearToGammaSpace(vec3 linRGB)
{
    linRGB = max(linRGB, vec3(0, 0, 0));
    // An almost-perfect approximation from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return max(1.055 * pow(linRGB, vec3(0.416666667)) - vec3(0.055), vec3(0));
}

void main() 
{
    mat3 TBN = mat3(inTangent, inBiTangent, inNormal);

    vec4 texAlbedoColor   = texture(texAlbedo, inUV);
    vec4 texNormalColor   = texture(texNormal, inUV);
    vec4 texORMParamColor = texture(texORMParam, inUV);

    vec3 albedo     = GammaToLinearSpace(texAlbedoColor.xyz);
    vec3 normal     = TBN * normalize(texNormalColor.xyz * 2.0 - 1.0);
    
    float occlusion = texORMParamColor.r;
    float roughness = texORMParamColor.g;
    float metallic  = texORMParamColor.b;

    vec4 finalColor = vec4(0, 0, 0, 1);

    // none
    if (uboParam.param.w == 0) {
        finalColor.xyz = texAlbedoColor.xyz;
    }
    // albedo
    else if (uboParam.param.w == 1) {
        finalColor.xyz = albedo.xyz;
    }
    // normal
    else if (uboParam.param.w == 2) {
        finalColor.xyz = normal.xyz;
    }
    // occlusion
    else if (uboParam.param.w == 3) {
        finalColor.xyz = vec3(occlusion);
    }
    // metallic
    else if (uboParam.param.w == 4) {
        finalColor.xyz = vec3(metallic);
    }
    // roughness
    else if (uboParam.param.w == 5) {
        finalColor.xyz = vec3(roughness);
    }

    outFragColor = finalColor;
}