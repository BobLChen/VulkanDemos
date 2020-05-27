#ifndef LIGHT_SAMPLING
#define LIGHT_SAMPLING

int GetLightId(RayPayloadInfo rayInfo)
{
	return -1;
}

bool IsEnvironmentLight(int lightId)
{
	return lights.datas[lightId].type.x == LIGHT_TYPE_SKY;
}

bool IsPointLight(int lightId)
{
	return lights.datas[lightId].type.x == LIGHT_TYPE_POINT;
}

bool IsDirectionalLight(int lightId)
{
	return lights.datas[lightId].type.x == LIGHT_TYPE_DIRECTIONAL;
}

bool IsRectLight(int lightId)
{
	return lights.datas[lightId].type.x == LIGHT_TYPE_RECT;
}

bool IsSpotLight(int lightId)
{
	return lights.datas[lightId].type.x == LIGHT_TYPE_SPOT;
}

vec3 GetLightUV(int lightId, RayDesc ray, RayPayloadInfo rayInfo)
{
	vec3 lightUV = vec3(0.0, 0.0, 0.0);
	if (IsEnvironmentLight(lightId)) {
		lightUV = ray.direction;
	}

	return lightUV;
}

vec3 GetPosition(int lightId)
{
    return lights.datas[lightId].position.xyz;
}

vec3 GetNormal(int lightId)
{
    return lights.datas[lightId].normal.xyz;
}

vec3 GetdPdu(int lightId)
{
    return lights.datas[lightId].dPdu.xyz;
}

vec3 GetdPdv(int lightId)
{
    return lights.datas[lightId].dPdv.xyz;
}

float GetWidth(int lightId)
{
    return lights.datas[lightId].dimensions.x;
}

float GetHeight(int lightId)
{
    return lights.datas[lightId].dimensions.y;
}

float GetDepth(int lightId)
{
    return lights.datas[lightId].dimensions.z;
}

float GetCosConeAngle(int lightId)
{
    return lights.datas[lightId].dimensions.x;
}

vec3 GetColor(int lightId)
{
    return lights.datas[lightId].color.xyz;
}

float GetRadius(int lightId)
{
    return lights.datas[lightId].dimensions.z;
}

float GetAttenuation(int lightId)
{
    return lights.datas[lightId].attenuation.x;
}

float GetRectLightBarnCosAngle(int lightId)
{
    return lights.datas[lightId].attenuation.x;
}

float GetRectLightBarnLength(int lightId)
{
    return lights.datas[lightId].attenuation.x;
}

float ComputeAttenuationFalloff(float distance, float attenuation)
{
	return 1.0;
}

#include "light/SkyLight.glsl"
#include "light/DirectionalLight.glsl"
#include "light/PointLight.glsl"
#include "light/RectLight.glsl"
#include "light/SpotLight.glsl"

bool SampleLightSelection(RayDesc ray, RayPayloadInfo payLoad, float randSample, inout int outLightId, inout float outLightSelectionPdf)
{
	outLightId = int(randSample * globalParam.lightInfo.x);
	outLightSelectionPdf = 1.0 / globalParam.lightInfo.x;
	return true;
}

void PdfLightSelection(RayDesc ray, RayPayloadInfo payload, int lightId, inout float outLightSelectionPdf)
{
	outLightSelectionPdf = 1.0 / globalParam.lightInfo.x;
	return;
}

void PdfLight(RayDesc ray, RayPayloadInfo payload, int lightId, vec3 lightUV, inout float outPdf)
{
	float lightSelectionPdf = 1.0 / globalParam.lightInfo.x;
	PdfLightSelection(ray, payload, lightId, lightSelectionPdf);

	float lightPdf = 0.0;
	if (IsEnvironmentLight(lightId))
	{
		SkyLightPdfLight(ray, lightId, lightUV, lightPdf);
	}
	else if (IsDirectionalLight(lightId))
	{
		DirectionalLightPdfLight(ray, lightId, lightUV, lightPdf);
	}

	outPdf = lightSelectionPdf * lightPdf;
}

void SampleLight(RayDesc ray, RayPayloadInfo payload, vec4 randSample, inout int outLightId, inout vec3 outLightUV, inout float outPdf)
{
	outPdf = 0.0f;

	float lightSelectionPdf = 0.0;
	if (!SampleLightSelection(ray, payload, randSample.x, outLightId, lightSelectionPdf)) {
		return;
	}

	float lightPdf = 0.0;
	if (IsDirectionalLight(outLightId))
	{
		DirectionalLightSampleLight(ray, payload, randSample.xyz, outLightId, outLightUV, lightPdf);
	}
	else if (IsEnvironmentLight(outLightId))
	{
		SkyLightSampleLight(ray, payload, randSample, outLightId, outLightUV, lightPdf);
	}

	outPdf = lightSelectionPdf * lightPdf;
}

void GenerateLightRay(RayDesc ray, int lightId, vec3 lightUV, inout RayDesc outLightRay)
{
	if (IsDirectionalLight(lightId))
	{
		DirectionalLightGenerateLightRay(ray, lightId, lightUV, outLightRay);
	}
	else if (IsEnvironmentLight(lightId))
	{
		SkyLightGenerateLightRay(ray, lightId, lightUV, outLightRay);
	}
}

void EvalLight(int lightId, vec3 lightUV, RayDesc ray, inout vec3 outRadiance)
{
	outRadiance = vec3(0.0f, 0.0f, 0.0f);

	if (IsEnvironmentLight(lightId))
	{
		SkyLightEvalLight(lightId, lightUV, ray, outRadiance);
	}
	else if (IsDirectionalLight(lightId))
	{
		DirectionalLightEvalLight(lightId, lightUV, ray, outRadiance);
	}
}

#endif