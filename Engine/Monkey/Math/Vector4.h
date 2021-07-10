#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Utils/StringUtils.h"

#include "Math.h"
#include "Vector3.h"
#include "Vector2.h"
#include "Color.h"

#include <string>

struct Vector4
{
public:
	float x;
	float y;
	float z;
	float w;

public:
    
	Vector4(const Vector3& inVector, float inW = 1.0f);

	Vector4(const LinearColor& inColor);

	explicit Vector4(float inX = 0.0f, float inY = 0.0f, float inZ = 0.0f, float inW = 1.0f);

	explicit Vector4(Vector2 inXY, Vector2 inZW);

	FORCE_INLINE float& operator[](int32 index);

	FORCE_INLINE float operator[](int32 index) const;

	FORCE_INLINE Vector4 operator-() const;

	FORCE_INLINE Vector4 operator+(const Vector4& v) const;

	FORCE_INLINE Vector4 operator+=(const Vector4& v);

	FORCE_INLINE Vector4 operator-(const Vector4& v) const;

	FORCE_INLINE Vector4 operator-=(const Vector4& v);

	FORCE_INLINE Vector4 operator*(float scale) const;

	FORCE_INLINE Vector4 operator/(float scale) const;

	FORCE_INLINE Vector4 operator/(const Vector4& v) const;

	FORCE_INLINE Vector4 operator*(const Vector4& v) const;

	FORCE_INLINE Vector4 operator*=(const Vector4& v);

	FORCE_INLINE Vector4 operator/=(const Vector4& v);

	FORCE_INLINE Vector4 operator*=(float f);

	FORCE_INLINE bool operator==(const Vector4& v) const;

	FORCE_INLINE bool operator!=(const Vector4& v) const;

	FORCE_INLINE Vector4 operator^(const Vector4& v) const;

	FORCE_INLINE float& Component(int32 index);

	FORCE_INLINE const float& Component(int32 index) const;

	FORCE_INLINE bool Equals(const Vector4& v, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE bool IsUnit3(float LengthSquaredTolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE std::string ToString() const;

	FORCE_INLINE Vector4 GetSafeNormal(float tolerance = SMALL_NUMBER) const;

	FORCE_INLINE Vector4 GetUnsafeNormal3() const;

	FORCE_INLINE void Set(float inX, float inY, float inZ, float inW);

	FORCE_INLINE float Size3() const;

	FORCE_INLINE float SizeSquared3() const;

	FORCE_INLINE float Size() const;

	FORCE_INLINE float SizeSquared() const;

	FORCE_INLINE bool ContainsNaN() const;

	FORCE_INLINE bool IsNearlyZero3(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE Vector4 Reflect3(const Vector4& normal) const;

	FORCE_INLINE void FindBestAxisVectors3(Vector4& axis1, Vector4& axis2) const;

	FORCE_INLINE static Vector4 Min(const Vector4& a, const Vector4& b);

	FORCE_INLINE static Vector4 Max(const Vector4& a, const Vector4& b);

	FORCE_INLINE void DiagnosticCheckNaN() 
	{

	}

	friend FORCE_INLINE float Dot3(const Vector4& v1, const Vector4& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	friend FORCE_INLINE float Dot4(const Vector4& v1, const Vector4& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
	}

	friend FORCE_INLINE Vector4 operator*(float scale, const Vector4& v)
	{
		return v.operator*(scale);
	}
};

FORCE_INLINE Vector4::Vector4(const Vector3& inVector, float inW)
	: x(inVector.x)
	, y(inVector.y)
	, z(inVector.z)
	, w(inW)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Vector4::Vector4(const LinearColor& InColor)
	: x(InColor.r)
	, y(InColor.g)
	, z(InColor.b)
	, w(InColor.a)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Vector4::Vector4(float inX, float inY, float inZ, float inW)
	: x(inX)
	, y(inY)
	, z(inZ)
	, w(inW)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE Vector4::Vector4(Vector2 inXY, Vector2 inZW)
	: x(inXY.x)
	, y(inXY.y)
	, z(inZW.x)
	, w(inZW.y)
{
	DiagnosticCheckNaN();
}

FORCE_INLINE float& Vector4::operator[](int32 index)
{
	return (&x)[index];
}

FORCE_INLINE float Vector4::operator[](int32 index) const
{
	return (&x)[index];
}

FORCE_INLINE void Vector4::Set(float inX, float inY, float inZ, float inW)
{
	x = inX;
	y = inY;
	z = inZ;
	w = inW;
	DiagnosticCheckNaN();
}

FORCE_INLINE Vector4 Vector4::operator-() const
{
	return Vector4(-x, -y, -z, -w);
}

FORCE_INLINE Vector4 Vector4::operator+(const Vector4& v) const
{
	return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

FORCE_INLINE Vector4 Vector4::operator+=(const Vector4& v)
{
	x += v.x; 
	y += v.y; 
	z += v.z; 
	w += v.w;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector4 Vector4::operator-(const Vector4& v) const
{
	return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

FORCE_INLINE Vector4 Vector4::operator-=(const Vector4& v)
{
	x -= v.x; 
	y -= v.y; 
	z -= v.z; 
	w -= v.w;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector4 Vector4::operator*(float scale) const
{
	return Vector4(x * scale, y * scale, z * scale, w * scale);
}

FORCE_INLINE Vector4 Vector4::operator/(float scale) const
{
	const float invScale = 1.f / scale;
	return Vector4(x * invScale, y * invScale, z * invScale, w * invScale);
}

FORCE_INLINE Vector4 Vector4::operator*(const Vector4& v) const
{
	return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
}

FORCE_INLINE Vector4 Vector4::operator^(const Vector4& v) const
{
	return Vector4(
		y * v.z - z * v.y,
		z * v.x - x * v.z,
		x * v.y - y * v.x,
		0.0f
	);
}

FORCE_INLINE float& Vector4::Component(int32 index)
{
	return (&x)[index];
}

FORCE_INLINE const float& Vector4::Component(int32 index) const
{
	return (&x)[index];
}

FORCE_INLINE bool Vector4::operator==(const Vector4& v) const
{
	return ((x == v.x) && (y == v.y) && (z == v.z) && (w == v.w));
}

FORCE_INLINE bool Vector4::operator!=(const Vector4& v) const
{
	return ((x != v.x) || (y != v.y) || (z != v.z) || (w != v.w));
}

FORCE_INLINE bool Vector4::Equals(const Vector4& v, float tolerance) const
{
	return MMath::Abs(x - v.x) <= tolerance && MMath::Abs(y - v.y) <= tolerance && MMath::Abs(z - v.z) <= tolerance && MMath::Abs(w - v.w) <= tolerance;
}

FORCE_INLINE std::string Vector4::ToString() const
{
	return StringUtils::Printf("x=%3.3f y=%3.3f z=%3.3f w=%3.3f", x, y, z, w);
}

FORCE_INLINE Vector4 Vector4::GetSafeNormal(float tolerance) const
{
	const float squareSum = x * x + y * y + z * z;
	if (squareSum > tolerance)
	{
		const float scale = MMath::InvSqrt(squareSum);
		return Vector4(x * scale, y * scale, z * scale, 0.0f);
	}
	return Vector4(0.f);
}

FORCE_INLINE Vector4 Vector4::GetUnsafeNormal3() const
{
	const float scale = MMath::InvSqrt(x * x + y * y + z * z);
	return Vector4(x * scale, y * scale, z * scale, 0.0f);
}

FORCE_INLINE float Vector4::Size3() const
{
	return MMath::Sqrt(x * x + y * y + z * z);
}

FORCE_INLINE float Vector4::SizeSquared3() const
{
	return x * x + y * y + z * z;
}

FORCE_INLINE float Vector4::Size() const
{
	return MMath::Sqrt(x * x + y * y + z * z + w * w);
}

FORCE_INLINE float Vector4::SizeSquared() const
{
	return x * x + y * y + z * z + w * w;
}

FORCE_INLINE bool Vector4::IsUnit3(float LengthSquaredTolerance) const
{
	return MMath::Abs(1.0f - SizeSquared3()) < LengthSquaredTolerance;
}

FORCE_INLINE bool Vector4::ContainsNaN() const
{
	return (
		!MMath::IsFinite(x) ||
		!MMath::IsFinite(y) ||
		!MMath::IsFinite(z) ||
		!MMath::IsFinite(w)
	);
}

FORCE_INLINE bool Vector4::IsNearlyZero3(float tolerance) const
{
	return
		MMath::Abs(x) <= tolerance && 
		MMath::Abs(y) <= tolerance && 
		MMath::Abs(z) <= tolerance;
}

FORCE_INLINE Vector4 Vector4::Reflect3(const Vector4& normal) const
{
	return 2.0f * Dot3(*this, normal) * normal - *this;
}

FORCE_INLINE Vector4 Vector4::Min(const Vector4& a, const Vector4& b)
{
	Vector4 result;
	result.x = MMath::Min(a.x, b.x);
	result.y = MMath::Min(a.y, b.y);
	result.z = MMath::Min(a.z, b.z);
	result.w = MMath::Min(a.w, b.w);
	return result;
}

FORCE_INLINE Vector4 Vector4::Max(const Vector4& a, const Vector4& b)
{
	Vector4 result;
	result.x = MMath::Max(a.x, b.x);
	result.y = MMath::Max(a.y, b.y);
	result.z = MMath::Max(a.z, b.z);
	result.w = MMath::Max(a.w, b.w);
	return result;
}

FORCE_INLINE void Vector4::FindBestAxisVectors3(Vector4& axis1, Vector4& axis2) const
{
	const float nx = MMath::Abs(x);
	const float ny = MMath::Abs(y);
	const float nz = MMath::Abs(z);

	if (nz > nx && nz > ny)
	{
		axis1 = Vector4(1, 0, 0);
	}
	else 
	{
		axis1 = Vector4(0, 0, 1);
	}

	axis1 = (axis1 - *this * Dot3(axis1, *this)).GetSafeNormal();
	axis2 = axis1 ^ *this;
}

FORCE_INLINE Vector4 Vector4::operator*=(const Vector4& v)
{
	x *= v.x; 
	y *= v.y; 
	z *= v.z; 
	w *= v.w;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector4 Vector4::operator/=(const Vector4& v)
{
	x /= v.x; 
	y /= v.y; 
	z /= v.z;
	w /= v.w;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector4 Vector4::operator*=(float f)
{
	x *= f; 
	y *= f; 
	z *= f; 
	w *= f;
	DiagnosticCheckNaN();
	return *this;
}

FORCE_INLINE Vector4 Vector4::operator/(const Vector4& v) const
{
	return Vector4(x / v.x, y / v.y, z / v.z, w / v.w);
}