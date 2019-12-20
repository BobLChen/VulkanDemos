#pragma once

#include "Common/Common.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

struct Ray;
struct HitInfo;

class Material
{
public:

	virtual bool Scatter(const Ray& ray, const HitInfo& hitInfo, Vector4& attenuation, Ray& reflect) const = 0;

protected:

	const Vector3 RandomUnit() const
	{
		Vector3 vec3(MMath::FRandRange(-1.0f, 1.0f), MMath::FRandRange(-1.0f, 1.0f), MMath::FRandRange(-1.0f, 1.0f));
		vec3 = vec3.GetSafeNormal();
		return vec3;
	}

};

class DiffuseMaterial : public Material
{
public:

	DiffuseMaterial(const Vector4& inAlbedo)
		: albedo(inAlbedo)
	{

	}

	bool Scatter(const Ray& ray, const HitInfo& hitInfo, Vector4& attenuation, Ray& reflect) const override;

	Vector4 albedo;

protected:

};


class MetalMaterial : public Material
{
public:

	MetalMaterial(const Vector4& inAlbedo, float inRoughness)
		: albedo(inAlbedo)
		, roughness(inRoughness)
	{

	}

	bool Scatter(const Ray& ray, const HitInfo& hitInfo, Vector4& attenuation, Ray& reflect) const override;

	Vector4 albedo;

	float roughness = 0.0f;

protected:

};