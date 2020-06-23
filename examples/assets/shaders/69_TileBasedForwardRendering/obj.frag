#version 450
#extension GL_ARB_separate_shader_objects : enable

#define LIGHT_SIZE 512
#define TILE_SIZE 16
#define LIGHT_SIZE_PER_TILE 63

#define PI 3.14159265359
#define saturate(x) clamp(x, 0.0, 1.0)

struct PointLight 
{
	vec3 pos;
	float radius;

	vec3 color;
    float padding;
};

struct LightVisiblity
{
	uint count;
	uint lightIndices[LIGHT_SIZE_PER_TILE];
};

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inWorldPos;

layout(binding = 1) buffer readonly LightsCullingBuffer
{
    LightVisiblity lightVisiblities[];
} lightsCullingBuffer;

layout (binding = 2) uniform LightsBlock 
{
    vec4 lightCount;
	PointLight lights[LIGHT_SIZE];
} uboLights;

layout (binding = 3) uniform CullingBlock 
{
    mat4 invProj;
    vec4 frameSize;
    vec4 tileNum;
    vec4 pos;
} uboCulling;

layout (binding = 4) uniform DebugBlock 
{
    vec4 param;
} uboDebug;

layout (location = 0) out vec4 outFragColor;

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
    ivec2 screen = ivec2(gl_FragCoord.xy);
    screen.y = int(uboCulling.frameSize.y - screen.y); // flip y
    
    ivec2 tileID = ivec2(screen.xy / TILE_SIZE);
    uint tileIndex = tileID.y * uint(uboCulling.tileNum.x) + tileID.x;

    vec3 color = vec3(0.0, 0.0, 0.0);
    vec3 diffuse = vec3(1.0, 1.0, 1.0);

    uint lightNum = lightsCullingBuffer.lightVisiblities[tileIndex].count;
    if (uboDebug.param.x == 1) 
    {
        outFragColor = vec4((inNormal + 1.0) * 0.5, 1.0);
        return;
    }
    else if (uboDebug.param.x == 2) 
    {
        outFragColor = vec4(float(lightNum) / float(LIGHT_SIZE_PER_TILE));
        return;
    }

    for (int i = 0; i < lightNum; ++i)
	{
        PointLight light = uboLights.lights[lightsCullingBuffer.lightVisiblities[tileIndex].lightIndices[i]];

        float dist = distance(light.pos, inWorldPos);
        if (dist > light.radius) {
            continue;
        }

        vec3 N = inNormal;
        vec3 L = normalize(light.pos - inWorldPos);
        vec3 V = normalize(uboCulling.pos.xyz - inWorldPos.xyz);
        vec3 H = normalize(L + V);

        float roughness = 0.5;
        float metallic  = 0.5;
        vec3 albedo = diffuse;

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

        vec3 wi = (KD * albedo / PI + brdf) * max(dot(N, L), 0) * light.color.xyz;

        float att = clamp(1.0 - dist * dist / (light.radius * light.radius), 0.0, 1.0);

        color += wi * att;
	}

    color.xyz = ACESFitted(color.xyz);
    color.xyz = pow(color.xyz, vec3(1.0 / 2.2));
    color.xyz = saturate(color.xyz);

    outFragColor = vec4(color, 1.0);
}