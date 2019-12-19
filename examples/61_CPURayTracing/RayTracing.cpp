#include "RayTracing.h"

HitInfo Sphere::HitTest(const Vector3& start, const Vector3& ray)
{
	Vector3 oc = start - center;
	float b = 2.0f * Vector3::DotProduct(oc, ray);
	float c = Vector3::DotProduct(oc, oc) - radius * radius;
	float h = b * b - 4.0f * c;

	HitInfo hitInfo;
	hitInfo.hit = false;

	if (h < 0.0f) {
		return hitInfo;
	}

	float t = (-b - MMath::Sqrt(h)) / 2.0f;

	hitInfo.hit    = t > EPSILON;
	hitInfo.dist   = t;
	hitInfo.inside = false;
	hitInfo.pos    = start + ray * t;
	hitInfo.normal = (hitInfo.pos - center) / radius;

	return hitInfo;
}

Vector4 Raytracing::HitScene()
{
	color.Set(0, 0, 0, 0);

	// inv transform
	Matrix4x4 invProj = camera->GetProjection();
	invProj.SetInverse();
	Matrix4x4 invView = camera->GetView();
	invView.SetInverse();

	for (int32 i = 0; i < 15; ++i)
	{
		// clip space
		Vector2 clip = Vector2((w + MMath::FRandRange(0.0f, 1.0f)) / width, (h + MMath::FRandRange(0.0f, 1.0f)) / height);
		// camera position
		Vector3 pos  = camera->GetTransform().GetOrigin();
		// clip space ray
		Vector3 ray  = Vector3(clip.x * 2.0 - 1.0, -(clip.y * 2.0 - 1.0), 1.0);
		// clip space to viewspace
		ray = invProj.TransformPosition(ray);
		ray.x = ray.x * ray.z;
		ray.y = ray.y * ray.z;
		// view space to world space
		ray = invView.TransformVector(ray);
		ray.Normalize();

		color += RayHitScene(pos, ray);
	}

	color = color / 15;

	return color;
}

HitInfo Raytracing::IntersectScene(Scene* scene, const Vector3& start, const Vector3& ray)
{
	HitInfo info;
	info.dist = MAX_int32;
	info.hit  = false;

	for (int32 i = 0; i < scene->spheres.size(); ++i)
	{
		Sphere& sphere  = scene->spheres[i];
		HitInfo tempHit = sphere.HitTest(start, ray);

		if (tempHit.hit && tempHit.dist < info.dist)
		{
			info = tempHit;
		}
	}

	return info;
}

Vector4 Raytracing::RayHitScene(const Vector3& start, const Vector3& ray)
{
	HitInfo hitInfo = IntersectScene(scene, start, ray);
	if (hitInfo.hit)
	{
		Vector3 newRay(MMath::FRandRange(-1.0f, 1.0f), MMath::FRandRange(-1.0f, 1.0f), MMath::FRandRange(-1.0f, 1.0f));
		newRay = newRay + hitInfo.normal;
		newRay.Normalize();

		return 0.5f * RayHitScene(hitInfo.pos, newRay);
	}
	else 
	{
		float t = (ray.y + 1.0f) * 0.5f;
		return (1.0f - t) * Vector4(1.0f, 1.0f, 1.0f, 1.0f) + t * Vector4(0.5f, 0.7f, 1.0f, 1.0f);
	}
}