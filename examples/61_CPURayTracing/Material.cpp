#include "Material.h"
#include "RayTracing.h"

bool DiffuseMaterial::Scatter(const Ray& ray, const HitInfo& hitInfo, Vector4& attenuation, Ray& reflect) const
{
	attenuation = albedo;
	
	reflect.start = hitInfo.pos;
	reflect.direction = (hitInfo.normal + RandomUnit()).GetSafeNormal();

	return true;
}

bool MetalMaterial::Scatter(const Ray& ray, const HitInfo& hitInfo, Vector4& attenuation, Ray& reflect) const
{
	attenuation = albedo;

	reflect.direction = ray.direction - 2 * hitInfo.normal * Vector3::DotProduct(ray.direction, hitInfo.normal); 
	reflect.direction = (reflect.direction + RandomUnit() * roughness).GetSafeNormal();
	reflect.start = hitInfo.pos;

	return Vector3::DotProduct(reflect.direction, hitInfo.normal) != 0;
}