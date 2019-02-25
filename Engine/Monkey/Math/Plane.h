#pragma once

#include "Vector.h"
#include "Vector4.h"
#include "Math.h"

struct Matrix4x4;

struct Plane : public Vector
{
public:
	float w;

public:

	FORCEINLINE Plane();

	FORCEINLINE Plane(const Plane& p);

	FORCEINLINE Plane(const Vector4& v);

	FORCEINLINE Plane(float inX, float inY, float inZ, float inW);

	FORCEINLINE Plane(Vector inNormal, float inW);

	FORCEINLINE Plane(Vector inBase, const Vector &inNormal);

	Plane(Vector a, Vector b, Vector c);

	FORCEINLINE float PlaneDot(const Vector &p) const;

	bool Normalize(float tolerance = SMALL_NUMBER);
	
	Plane Flip() const;

	Plane TransformBy(const Matrix4x4& m) const;

	Plane TransformByUsingAdjointT(const Matrix4x4& m, float detM, const Matrix4x4& ta) const;

	bool operator==(const Plane& v) const;

	bool operator!=(const Plane& v) const;

	bool Equals(const Plane& v, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE float operator|(const Plane& v) const;

	Plane operator+(const Plane& v) const;

	Plane operator-(const Plane& v) const;

	Plane operator/(float scale) const;

	Plane operator*(float scale) const;

	Plane operator*(const Plane& v);

	Plane operator+=(const Plane& v);

	Plane operator-=(const Plane& v);

	Plane operator*=(float scale);

	Plane operator*=(const Plane& v);

	Plane operator/=(float v);

};

inline Vector Vector::MirrorByPlane(const Plane& plane) const
{
	return *this - plane * (2.f * plane.PlaneDot(*this));
}

inline Vector Vector::PointPlaneProject(const Vector& point, const Plane& plane)
{
	return point - plane.PlaneDot(point) * plane;
}

inline Vector Vector::PointPlaneProject(const Vector& point, const Vector& a, const Vector& b, const Vector& c)
{
	Plane plane(a, b, c);
	return point - plane.PlaneDot(point) * plane;
}

FORCEINLINE Plane::Plane()
{

}


FORCEINLINE Plane::Plane(const Plane& p)
	: Vector(p)
	, w(p.w)
{

}

FORCEINLINE Plane::Plane(const Vector4& v)
	: Vector(v)
	, w(v.w)
{

}

FORCEINLINE Plane::Plane(float inX, float inY, float inZ, float inW)
	: Vector(inX, inY, inZ)
	, w(inW)
{

}

FORCEINLINE Plane::Plane(Vector inNormal, float inW)
	: Vector(inNormal)
	, w(inW)
{

}

FORCEINLINE Plane::Plane(Vector inBase, const Vector &inNormal)
	: Vector(inNormal)
	, w(inBase | inNormal)
{

}

FORCEINLINE Plane::Plane(Vector a, Vector b, Vector c)
	: Vector(((b - a) ^ (c - a)).GetSafeNormal())
{
	w = a | (Vector)(*this);
}

FORCEINLINE float Plane::PlaneDot(const Vector &p) const
{
	return x * p.x + y * p.y + z * p.z - w;
}

FORCEINLINE bool Plane::Normalize(float tolerance)
{
	const float squareSum = x * x + y * y + z * z;
	if (squareSum > tolerance)
	{
		const float scale = MMath::InvSqrt(squareSum);
		x *= scale; 
		y *= scale; 
		z *= scale;
		w *= scale;
		return true;
	}
	return false;
}

FORCEINLINE Plane Plane::Flip() const
{
	return Plane(-x, -y, -z, -w);
}

FORCEINLINE bool Plane::operator==(const Plane& v) const
{
	return (x == v.x) && (y == v.y) && (z == v.z) && (w == v.w);
}

FORCEINLINE bool Plane::operator!=(const Plane& v) const
{
	return (x != v.x) || (y != v.y) || (z != v.z) || (w != v.w);
}

FORCEINLINE bool Plane::Equals(const Plane& v, float tolerance) const
{
	return (MMath::Abs(x - v.x) < tolerance) && (MMath::Abs(y - v.y) < tolerance) && (MMath::Abs(z - v.z) < tolerance) && (MMath::Abs(w - v.w) < tolerance);
}

FORCEINLINE float Plane::operator|(const Plane& v) const
{
	return x * v.x + y * v.y + z * v.z + w * v.w;
}

FORCEINLINE Plane Plane::operator+(const Plane& v) const
{
	return Plane(x + v.x, y + v.y, z + v.z, w + v.w);
}

FORCEINLINE Plane Plane::operator-(const Plane& v) const
{
	return Plane(x - v.x, y - v.y, z - v.z, w - v.w);
}

FORCEINLINE Plane Plane::operator/(float scale) const
{
	const float RScale = 1.f / scale;
	return Plane(x * RScale, y * RScale, z * RScale, w * RScale);
}

FORCEINLINE Plane Plane::operator*(float scale) const
{
	return Plane(x * scale, y * scale, z * scale, w * scale);
}

FORCEINLINE Plane Plane::operator*(const Plane& v)
{
	return Plane(x * v.x, y * v.y, z * v.z, w * v.w);
}

FORCEINLINE Plane Plane::operator+=(const Plane& v)
{
	x += v.x; y += v.y; z += v.z; w += v.w;
	return *this;
}

FORCEINLINE Plane Plane::operator-=(const Plane& v)
{
	x -= v.x; y -= v.y; z -= v.z; w -= v.w;
	return *this;
}

FORCEINLINE Plane Plane::operator*=(float scale)
{
	x *= scale; y *= scale; z *= scale; w *= scale;
	return *this;
}

FORCEINLINE Plane Plane::operator*=(const Plane& v)
{
	x *= v.x; y *= v.y; z *= v.z; w *= v.w;
	return *this;
}

FORCEINLINE Plane Plane::operator/=(float v)
{
	const float RV = 1.f / v;
	x *= RV; y *= RV; z *= RV; w *= RV;
	return *this;
}
