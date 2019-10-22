#version 450

layout (binding = 1) uniform PBRParamBlock 
{
	vec4 param;
} uboParam;

layout (binding = 2) uniform samplerCube environmentMap;

layout (set = 0, location = 0) in vec3 inUVW;

layout (set = 0, location = 0) out vec4 outColor;

#define PI 3.14159265359

float RadicalInverseVdc(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i) / float(N), RadicalInverseVdc(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness) 
{
	float a = roughness * roughness;
	
	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
	vec3 H = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

	vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3 sampleVec = H.x * tangent + H.y * bitangent + H.z * N;
	return normalize(sampleVec);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float a = roughness;
	float k = (a * a) / 2.0;
	return NdotV / (NdotV * (1.0 - k) + k);
}

float NormalDistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a  = roughness * roughness;
	float a2 = a * a;

	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	float denom  = (NdotH2 * (a2 - 1.0) + 1.0);

	return a2 / (PI * denom * denom);
}

void main() 
{
	float roughness = uboParam.param.x;
	float envSize   = uboParam.param.y;

    vec3 N = normalize(inUVW);
	vec3 V = N;

	const uint sampleCount = 1024u;

	float totalWeight = 0.0;
	vec3 prefiltered  = vec3(0.0);

	for (uint i = 0u; i < sampleCount; ++i)
	{
		vec2 Xi = Hammersley(i, sampleCount);
		vec3 H  = ImportanceSampleGGX(Xi, N, roughness);
		vec3 L  = normalize(2.0 * dot(V, H) * H - V);
		float NdotL = max(dot(N, L), 0.0);

		if (NdotL > 0.0);
		{
			float NdotH = max(dot(N, H), 0.0);
			float VdotH = max(dot(V, H), 0.0);
			float pdf   = NormalDistributionGGX(N, H, roughness) * NdotH / (4.0 * VdotH) + 0.0001;

			float omegaS = 1.0 / (sampleCount * pdf);
			float omegaP = 4.0 * PI / (6.0 * envSize * envSize);

			float mipLevel = roughness == 0.0 ? 0.0 : max(0.5 * log2(omegaS / omegaP) + 1.0, 0.0);

			prefiltered += textureLod(environmentMap, L, mipLevel).xyz * NdotL;
			totalWeight += NdotL;
		}
	}

	prefiltered = prefiltered / totalWeight;
	outColor = vec4(prefiltered, 1.0);
}