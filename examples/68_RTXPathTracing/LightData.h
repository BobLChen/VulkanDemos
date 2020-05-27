#pragma once

#include "Math/Math.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

#define LIGHT_TYPE_SKY              0 
#define LIGHT_TYPE_POINT			1
#define LIGHT_TYPE_DIRECTIONAL		2
#define LIGHT_TYPE_RECT				3
#define LIGHT_TYPE_SPOT				4
#define LIGHT_TYPE_MAX				5 

class Light
{
public:
	Light(int inType)
		: type(inType)
	{

	}

	virtual ~Light()
	{

	}

	int type;
};

class DirectionalLight : public Light
{
public:

	DirectionalLight(const Vector3& inDirection, const Vector3& inColor)
		: Light(LIGHT_TYPE_DIRECTIONAL)
		, direction(inDirection)
		, color(inColor)
		, radius(MAX_int32)
	{

	}

	Vector3 direction;
	Vector3 color;
	float radius;
};

class RectLight : public Light
{
public:

	RectLight(
		const Vector3& inPosition, 
		const Vector3& inDirection, 
		const Vector3& inTangent, 
		const Vector3& inColor,
		float inSourceRadius,
		float inSourceLength,
		float inRadius,
		float inBarnCosAngle,
		float inBarnLength
	)
		: Light(LIGHT_TYPE_RECT)
		, position(inPosition)
		, direction(inDirection)
		, tangent(inTangent)
		, color(inColor)
		, sourceRadius(inSourceRadius)
		, sourceLength(inSourceLength)
		, radius(inRadius)
		, barnCosAngle(inBarnCosAngle)
		, barnLength(inBarnLength)
	{

	}

	Vector3 position;
	Vector3 direction;
	Vector3 tangent;
	Vector3 color;
	float sourceRadius;
	float sourceLength;
	float radius;
	float barnCosAngle;
	float barnLength;
};

class SpotLight : public Light
{
public:
	SpotLight(const Vector3& inPosition, const Vector3& inDirection, const Vector3& inColor, float inSpotAngles, float inSourceRadius, float inRadius)
		: Light(LIGHT_TYPE_SPOT)
		, position(inPosition)
		, direction(inDirection)
		, color(inColor)
		, spotAngles(inSpotAngles)
		, sourceRadius(inSourceRadius)
		, radius(inRadius)
	{

	}

	Vector3 position;
	Vector3 direction;
	Vector3 color;
	float spotAngles;
	float sourceRadius;
	float radius;
};

class PointLight : public Light
{
public:

	PointLight(const Vector3& inPosition, const Vector3& inColor, float inSrouceRadius, float inRadius)
		: Light(LIGHT_TYPE_POINT)
		, position(inPosition)
		, color(inColor)
		, sourceRadius(inSrouceRadius)
		, radius(inRadius)
	{

	}

	Vector3 position;
	Vector3 color;
	float sourceRadius;
	float radius;
};

class SkyEnvLight : public Light
{
public:
	SkyEnvLight(const Vector3& inColor)
		: Light(LIGHT_TYPE_SKY)
		, color(inColor)
	{

	}

	Vector3 color;
};

struct LightData
{
	Vector4 type;
	Vector4 position;
	Vector4 normal;
	Vector4 dPdu;
	Vector4 dPdv;
	Vector4 color;
	Vector4 dimensions;
	Vector4 attenuation;

	void SetLight(const DirectionalLight* light)
	{
		type.x = light->type;
		normal = -light->direction;
		color = light->color;
		attenuation.x = light->radius;
	}

	void SetLight(const RectLight* light)
	{
		type.x = light->type;
		position = light->position;
		normal = -light->direction;
		dPdu = Vector3::CrossProduct(light->tangent, light->direction);
		dPdv = light->tangent;
		color = light->color;
		dimensions.x = 2.0f * light->sourceRadius;
		dimensions.y = 2.0f * light->sourceLength;
		dimensions.z = light->barnCosAngle;
		dimensions.w = light->barnLength;
		attenuation.x = light->radius;
	}

	void SetLight(const SpotLight* light)
	{
		type.x = light->type;
		position = light->position;
		normal = -light->direction;
		color = 4.0f * PI * light->color;
		dimensions.x = light->spotAngles;
		dimensions.y = light->sourceRadius;
		attenuation.x = light->radius;
	}

	void SetLight(const PointLight* light)
	{
		type.x = light->type;
		position = light->position;
		color = light->color / (4.0f * PI);
		dimensions.x = 0.0f;
		dimensions.y = 0.0f;
		dimensions.z = light->sourceRadius;
		attenuation.x = light->radius;
	}

	void SetLight(const SkyEnvLight* light)
	{
		type.x = light->type;
		color = light->color;
	}
};