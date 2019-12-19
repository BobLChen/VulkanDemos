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

	HitInfo HitTest(const Vector3& start, const Vector3& ray);
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

	Vector4 HitScene();

	virtual void DoThreadedWork() override
	{
		HitScene();
	}

	virtual void Abandon() override
	{

	}

private:

	HitInfo IntersectScene(Scene* scene, const Vector3& start, const Vector3& ray);

	Vector4 RayHitScene(const Vector3& start, const Vector3& ray);

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
