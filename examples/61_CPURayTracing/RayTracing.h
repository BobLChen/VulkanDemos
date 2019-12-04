#pragma once

#include "Common/Common.h"
#include "Math/Vector3.h"
#include "Demo/DVKCamera.h"
#include "ThreadTask.h"

#define EPSILON 0.0001

struct HitInfo
{
	bool    hit = false;
	Vector3 pos;
	float   dist = -1;
	Vector3 normal;
	bool    inside = false;
};

struct Sphere
{
	Vector3 center;
	float   radius;

	Sphere(const Vector3& inCenter, float inRadius)
		: center(inCenter)
		, radius(inRadius)
	{

	}

	HitInfo HitTest(const Vector3& start, const Vector3& ray)
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
};

struct Scene
{
	std::vector<Sphere> spheres;
};

class Raytracing : public ThreadTask
{
public:

	Raytracing(Scene* inScene, vk_demo::DVKCamera* inCamera, float iw, float ih, int32 idx, int32 inWidth, int32 inHeight)
		: scene(inScene)
		, w(iw)
		, h(ih)
		, index(idx)
		, camera(inCamera)
		, width(inWidth)
		, height(inHeight)
	{

	}

	~Raytracing()
	{

	}

	Vector4 HitScene()
	{
		color.Set(0, 0, 0, 0);

		// inv transform
		Matrix4x4 invProj = camera->GetProjection();
		invProj.SetInverse();
		Matrix4x4 invView = camera->GetView();
		invView.SetInverse();
		// clip space
		Vector2 clip = Vector2(w / width, h / height);
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

		color = RayHitScene(pos, ray, 1.0f);

		return color;
	}

	virtual void DoThreadedWork() override
	{
		HitScene();
	}

	virtual void Abandon() override
	{

	}

private:

	HitInfo IntersectScene(Scene* scene, const Vector3& start, const Vector3& ray)
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

	Vector4 RayHitScene(const Vector3& start, const Vector3& ray, float energy)
	{
		HitInfo hitInfo = IntersectScene(scene, start, ray);
		if (hitInfo.hit)
		{

		}

		return (hitInfo.normal + 1.0f) * 0.5f;
	}

public:

	Scene* scene;
	float w;
	float h;
	int32 index;
	vk_demo::DVKCamera* camera;
	int32 width;
	int32 height;
	Vector4 color;
};
