#pragma once

#include "Common/Common.h"
#include "Math/Vector3.h"
#include "Demo/DVKCamera.h"
#include "ThreadTask.h"
#include "Material.h"

#define EPSILON 0.0001

struct HitInfo;
struct Ray;

struct Ray
{
	Vector3		start;
	Vector3		direction;
};

struct HitInfo
{
	bool		hit = false;
	Vector3		pos;
	float		dist = -1;
	Vector3		normal;
	bool		inside = false;
	Material*	material = nullptr;
};

struct Sphere
{
	Vector3		center;
	float		radius;
	Material*	material = nullptr;

	Sphere(const Vector3& inCenter, float inRadius, Material* inMaterial)
		: center(inCenter)
		, radius(inRadius)
		, material(inMaterial)
	{

	}

	HitInfo HitTest(const Ray& ray);
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
		, complete(false)
	{

	}

	~Raytracing()
	{

	}

	Vector4 HitScene();

	virtual void DoThreadedWork() override
	{
		HitScene();

		complete = true;
	}

	virtual void Abandon() override
	{

	}

private:

	HitInfo IntersectScene(Scene* scene, const Ray& ray);

	Vector4 RayHitScene(const Ray& ray, int32 depth);

public:

	Scene* scene;
	float w;
	float h;
	int32 index;
	vk_demo::DVKCamera* camera;
	int32 width;
	int32 height;
	Vector4 color;

	bool complete;
};
