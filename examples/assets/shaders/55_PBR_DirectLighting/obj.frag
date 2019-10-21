#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inTangent;
layout (location = 3) in vec3 inBiTangent;
layout (location = 4) in vec3 inWorldPos;

layout (binding = 1) uniform PBRParamBlock 
{
	vec4 param;
    vec4 cameraPos;
    vec4 lightColor;
} uboParam;

layout (binding  = 2) uniform sampler2D texAlbedo;
layout (binding  = 3) uniform sampler2D texNormal;
layout (binding  = 4) uniform sampler2D texORMParam;

layout (location = 0) out vec4 outFragColor;

#define PI 3.14159265359
#define saturate(x) clamp(x, 0.0, 1.0)

vec3 GammaToLinearSpace(vec3 sRGB)
{
    // Approximate version from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
}

vec3 LinearToGammaSpace(vec3 linRGB)
{
    linRGB = max(linRGB, vec3(0, 0, 0));
    // An almost-perfect approximation from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return max(1.055 * pow(linRGB, vec3(0.416666667)) - vec3(0.055), vec3(0));
}

float NormalDistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a  = roughness * roughness;
	float a2 = a * a;

	float NdotH  = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH * NdotH;
	float denom  = (NdotH2 * (a2 - 1.0f) + 1.0f);

	return a2 / (PI * denom * denom);
}

vec3 FresnelSchlick(vec3 H, vec3 V, vec3 F0)
{
    float HdotV = max(dot(H, V), 0.0f);
	return F0 + (1.0f - F0) * pow(1.0 - HdotV, 5.0f);
}

float GeometrySchlickGGX(float NdotV, float K)
{
	return NdotV / (NdotV * (1.0f - K) + K);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float K = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;

	float NdotV = max(dot(N, V), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);
	float ggx2  = GeometrySchlickGGX(NdotV, K);
	float ggx1  = GeometrySchlickGGX(NdotL, K);

	return ggx2 * ggx1;
}

const mat3 ACESInputMat = mat3(
    vec3(0.59719, 0.07600, 0.02840),
    vec3(0.35458, 0.90834, 0.13383),
    vec3(0.04823, 0.01566, 0.83777)
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat = mat3(
    vec3( 1.60475, -0.10208, -0.00327),
    vec3(-0.53108,  1.10813, -0.07276),
    vec3(-0.07367, -0.00605,  1.07602)
);

vec3 RRTAndODTFit(vec3 v)
{
    vec3 a = v * (v + 0.0245786) - 0.000090537;
    vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
    return a / b;
}

vec3 ACESFitted(vec3 color)
{
    color = ACESInputMat * color;

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = ACESOutputMat * color;

    // Clamp to [0, 1]
    color = saturate(color);

    return color;
}

void main() 
{
    mat3 TBN = mat3(inTangent, inBiTangent, inNormal);

    vec4 texAlbedoColor   = texture(texAlbedo, inUV);
    vec4 texNormalColor   = texture(texNormal, inUV);
    vec4 texORMParamColor = texture(texORMParam, inUV);

    vec3 albedo = GammaToLinearSpace(texAlbedoColor.xyz);
    vec3 normal = TBN * normalize(texNormalColor.xyz * 2.0 - 1.0);

    vec3 N = normal;
    vec3 L = vec3(0, 0, -1);
    vec3 V = normalize(uboParam.cameraPos.xyz - inWorldPos.xyz);
    vec3 H = normalize(L + V);
    
    float occlusion = texORMParamColor.r * uboParam.param.x;
    float roughness = texORMParamColor.g * uboParam.param.y;
    float metallic  = texORMParamColor.b * uboParam.param.z;

    // -------------------------
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    albedo = albedo * (1 - metallic) * (1 - 0.04);

    // F
    vec3  F = FresnelSchlick(H, V, F0);
    // D
    float D = NormalDistributionGGX(N, H, roughness);
    // G
    float G = GeometrySmith(N, V, L, roughness);
    // BRDF
    vec3 brdf = (D * F * G) / (4.0 * max(dot(V, N), 0) * max(dot(L, N), 0) + 0.0001f);
    // KD
    vec3 KD = (vec3(1.0f) - F);
    
    vec3 finalColor = (KD * albedo / PI + brdf) * max(dot(N, L), 0) * uboParam.lightColor.xyz * uboParam.lightColor.w;
    finalColor *= occlusion;

    // none
    if (uboParam.param.w == 0) {

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

    // tonemap
    finalColor.xyz = ACESFitted(finalColor.xyz);
    // linear to gamma
    finalColor.xyz = pow(finalColor.xyz, vec3(1.0 / 2.2));

    finalColor.xyz = saturate(finalColor.xyz);

    outFragColor.xyz = finalColor;
    outFragColor.w   = 1.0;
}