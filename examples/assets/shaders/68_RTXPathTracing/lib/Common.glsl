#ifndef COMMON
#define COMMON

#extension GL_EXT_control_flow_attributes : require

#define PI        3.14159265358979323
#define TWO_PI    6.28318530717958648
#define TMIN      0
#define TMAX      1.0e27

#define GLOSSY_MIN_ROUGHNESS	0.001f
#define GLOSSY_MAX_ROUGHNESS	0.999f

#define LIGHT_TYPE_SKY              0 
#define LIGHT_TYPE_POINT			1
#define LIGHT_TYPE_DIRECTIONAL		2
#define LIGHT_TYPE_RECT				3
#define LIGHT_TYPE_SPOT				4
#define LIGHT_TYPE_MAX				5

struct RayDesc
{
    vec3   origin;
    float  tMin;
    vec3   direction;
    float  tMax;
};

struct CameraInfo
{
    vec4 pos;
	mat4 invProj;
	mat4 invView;
	vec4 viewSize;
};

struct RayCone
{
	float width;
	float spreadAngle;
};

struct LightData 
{
    vec4 type;
	vec4 position;
	vec4 normal;
	vec4 dPdu;
	vec4 dPdv;
	vec4 color;
	vec4 dimensions;
	vec4 attenuation;
};

struct RayPayloadInfo
{
	float hitT;
	vec3 worldPos;
	vec3 worldNormal;
	vec3 radiance;
	vec3 baseColor;
	float roughness;
	float metallic;
	float ior;
	float opacity;
	float shadingMode;
	vec3 diffuseColor;
	vec3 specularColor;
};

layout(binding = 0, set = 0) uniform accelerationStructureNV topLevelAS;
layout(binding = 1, set = 0, rgba32f) uniform image2D image;
layout(binding = 2, set = 0) uniform GlobalParamBlock 
{
	vec4 pos;
	mat4 invProj;
	mat4 invView;
	vec4 samplingData;
	vec4 viewSize;
	vec4 lightInfo;
	vec4 moving;
} globalParam;

layout(std430, binding = 3, set = 0) buffer LightDatasBuffer
{
    LightData datas[];
} lights;

layout(location = 0) rayPayloadNV RayPayloadInfo payLoadInfo;

void ApplyPositionBias(inout RayDesc ray, const vec3 worldNormal, const float maxNormalBias)
{
	const float minBias = 0.01f;
	const float maxBias = max(minBias, maxNormalBias);
	const float normalBias = mix(maxBias, minBias, clamp(dot(worldNormal, ray.direction), 0.0, 1.0));

	ray.origin += worldNormal * normalBias;
}

#endif