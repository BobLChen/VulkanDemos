#include "RayTracing.h"

HitInfo Sphere::HitTest(const Ray& ray)
{
	Vector3 oc = ray.start - center;
	float b = 2.0f * Vector3::DotProduct(oc, ray.direction);
	float c = Vector3::DotProduct(oc, oc) - radius * radius;
	float h = b * b - 4.0f * c;

	HitInfo hitInfo;
	hitInfo.hit = false;

	if (h < 0.0f) {
		return hitInfo;
	}

	float t = (-b - MMath::Sqrt(h)) / 2.0f;

	hitInfo.hit      = t > EPSILON;
	hitInfo.dist     = t;
	hitInfo.inside   = false;
	hitInfo.pos      = ray.start + ray.direction * t;
	hitInfo.normal   = (hitInfo.pos - center) / radius;
	hitInfo.material = material;

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

	int32 sample = 10;

	for (int32 i = 0; i < sample; ++i)
	{
		// clip space
		Vector2 clip = Vector2((w + MMath::FRandRange(0.0f, 1.0f)) / width, (h + MMath::FRandRange(0.0f, 1.0f)) / height);
		// camera position
		Vector3 pos  = camera->GetTransform().GetOrigin();
		// clip space ray
		Vector3 dir  = Vector3(clip.x * 2.0 - 1.0, -(clip.y * 2.0 - 1.0), 1.0);
		// clip space to viewspace
		dir = invProj.TransformPosition(dir);
		dir.x = dir.x * dir.z;
		dir.y = dir.y * dir.z;
		// view space to world space
		dir = invView.TransformVector(dir);
		dir.Normalize();

		Ray ray;
		ray.start = pos;
		ray.direction = dir;
		
		color += RayHitScene(ray, 25);
	}

	color = color / 15;

	return color;
}

HitInfo Raytracing::IntersectScene(Scene* scene, const Ray& ray)
{
	HitInfo info;
	info.dist = MAX_int32;
	info.hit  = false;

	for (int32 i = 0; i < scene->spheres.size(); ++i)
	{
		Sphere& sphere  = scene->spheres[i];
		HitInfo tempHit = sphere.HitTest(ray);

		if (tempHit.hit && tempHit.dist < info.dist)
		{
			info = tempHit;
		}
	}

	return info;
}

Vector4 Raytracing::RayHitScene(const Ray& ray, int32 depth)
{
	HitInfo hitInfo = IntersectScene(scene, ray);

	if (hitInfo.hit)
	{
		if (depth <= 0) {
			return Vector4(0, 0, 0, 1.0);
		}

		Vector4 attenuation;
		Ray reflect;

		if (hitInfo.material->Scatter(ray, hitInfo, attenuation, reflect)) {
			return attenuation * RayHitScene(reflect, depth - 1);
		}
		else {
			return Vector4(0.1, 0.1, 0.1, 1.0);
		}
	}
	else 
	{
		float t = (ray.direction.y + 1.0f) * 0.5f;
		return (1.0f - t) * Vector4(1.0f, 1.0f, 1.0f, 1.0f) + t * Vector4(0.5f, 0.7f, 1.0f, 1.0f);
	}
}