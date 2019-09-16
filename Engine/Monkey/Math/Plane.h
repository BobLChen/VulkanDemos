#pragma once

#include "Vector3.h"
#include "Vector4.h"
#include "Math.h"

struct Matrix4x4;

struct Plane : public Vector3
{
public:
	float w;

public:

	Plane();

	Plane(const Plane& p);

	Plane(const Vector4& v);

	Plane(float inX, float inY, float inZ, float inW);

	Plane(Vector3 inNormal, float inW);

	Plane(Vector3 inBase, const Vector3 &inNormal);

	Plane(Vector3 a, Vector3 b, Vector3 c);

	FORCEINLINE float PlaneDot(const Vector3 &p) const;

	FORCEINLINE bool Normalize(float tolerance = SMALL_NUMBER);
	
	FORCEINLINE Plane Flip() const;

	FORCEINLINE Plane TransformBy(const Matrix4x4& m) const;

	FORCEINLINE Plane TransformByUsingAdjointT(const Matrix4x4& m, float detM, const Matrix4x4& ta) const;

	FORCEINLINE bool operator==(const Plane& v) const;

	FORCEINLINE bool operator!=(const Plane& v) const;

	FORCEINLINE bool Equals(const Plane& v, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCEINLINE float operator|(const Plane& v) const;

	FORCEINLINE Plane operator+(const Plane& v) const;

	FORCEINLINE Plane operator-(const Plane& v) const;

	FORCEINLINE Plane operator/(float scale) const;

	FORCEINLINE Plane operator*(float scale) const;

	FORCEINLINE Plane operator*(const Plane& v);

	FORCEINLINE Plane operator+=(const Plane& v);

	FORCEINLINE Plane operator-=(const Plane& v);

	FORCEINLINE Plane operator*=(float scale);

	FORCEINLINE Plane operator*=(const Plane& v);

	FORCEINLINE Plane operator/=(float v);

};

FORCEINLINE Plane::Plane()
{

}

FORCEINLINE Plane::Plane(const Plane& p)
	: Vector3(p)
	, w(p.w)
{

}

FORCEINLINE Plane::Plane(const Vector4& v)
	: Vector3(v)
	, w(v.w)
{

}

FORCEINLINE Plane::Plane(float inX, float inY, float inZ, float inW)
	: Vector3(inX, inY, inZ)
	, w(inW)
{

}

FORCEINLINE Plane::Plane(Vector3 inNormal, float inW)
	: Vector3(inNormal)
	, w(inW)
{

}

FORCEINLINE Plane::Plane(Vector3 inBase, const Vector3 &inNormal)
	: Vector3(inNormal)
	, w(inBase | inNormal)
{

}

FORCEINLINE Plane::Plane(Vector3 a, Vector3 b, Vector3 c)
	: Vector3(((b - a) ^ (c - a)).GetSafeNormal())
{
	w = a | (Vector3)(*this);
}

FORCEINLINE float Plane::PlaneDot(const Vector3 &p) const
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
	x += v.x; 
	y += v.y; 
	z += v.z; 
	w += v.w;
	return *this;
}

FORCEINLINE Plane Plane::operator-=(const Plane& v)
{
	x -= v.x; 
	y -= v.y; 
	z -= v.z; 
	w -= v.w;
	return *this;
}

FORCEINLINE Plane Plane::operator*=(float scale)
{
	x *= scale; 
	y *= scale; 
	z *= scale; 
	w *= scale;
	return *this;
}

FORCEINLINE Plane Plane::operator*=(const Plane& v)
{
	x *= v.x; 
	y *= v.y; 
	z *= v.z; 
	w *= v.w;
	return *this;
}

FORCEINLINE Plane Plane::operator/=(float v)
{
	const float rv = 1.f / v;
	x *= rv; 
	y *= rv; 
	z *= rv; 
	w *= rv;
	return *this;
}
