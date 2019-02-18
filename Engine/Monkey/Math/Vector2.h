#pragma once

#include "Common/Common.h"
#include "Math/IntPoint.h"

struct Vector2
{
	float x;
	float y;

	static const Vector2 ZeroVector;
	static const Vector2 UnitVector;

	FORCEINLINE Vector2();

	FORCEINLINE Vector2(float inX, float inY);

	FORCEINLINE Vector2(IntPoint inPos);

	FORCEINLINE Vector2 operator+(const Vector2& v) const;

	FORCEINLINE Vector2 operator-(const Vector2& v) const;

	FORCEINLINE Vector2 operator*(float scale) const;

	FORCEINLINE Vector2 operator+(float a) const;

	FORCEINLINE Vector2 operator-(float a) const;

	FORCEINLINE Vector2 operator*(const Vector2& v) const;

	Vector2 operator/(float scale) const;

	Vector2 operator/(const Vector2& v) const;

	FORCEINLINE float operator|(const Vector2& v) const;

	FORCEINLINE float operator^(const Vector2& v) const;

	bool operator==(const Vector2& v) const;

	bool operator!=(const Vector2& v) const;

	bool operator<(const Vector2& other) const;

	bool operator>(const Vector2& other) const;

	bool operator<=(const Vector2& other) const;

	bool operator>=(const Vector2& other) const;

	FORCEINLINE Vector2 operator-() const;

	FORCEINLINE Vector2 operator+=(const Vector2& v);

	FORCEINLINE Vector2 operator-=(const Vector2& v);

	FORCEINLINE Vector2 operator*=(float scale);

	Vector2 operator/=(float v);

	Vector2 operator*=(const Vector2& v);

	Vector2 operator/=(const Vector2& v);

	float& operator[](int32 index);

	float operator[](int32 index) const;

	float& Component(int32 index);

	float Component(int32 index) const;

	FORCEINLINE static float DotProduct(const Vector2& a, const Vector2& b);

	FORCEINLINE static float DistSquared(const Vector2& v1, const Vector2& v2);

	FORCEINLINE static float Distance(const Vector2& v1, const Vector2& v2);

	FORCEINLINE static float CrossProduct(const Vector2& a, const Vector2& b);

	bool Equals(const Vector2& v, float tolerance = KINDA_SMALL_NUMBER) const;

	void Set(float inX, float inY);

	float GetMax() const;

	float GetAbsMax() const;

	float GetMin() const;

	float Size() const;

	float SizeSquared() const;

	Vector2 GetRotated(float angleDeg) const;

	Vector2 GetSafeNormal(float tolerance = SMALL_NUMBER) const;

	void Normalize(float tolerance = SMALL_NUMBER);

	bool IsNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const;

	void ToDirectionAndLength(Vector2 &outDir, float &outLength) const;

	bool IsZero() const;

	IntPoint GetIntPoint() const;

	Vector2 RoundToVector() const;

	Vector2 ClampAxes(float minAxisVal, float maxAxisVal) const;

	FORCEINLINE Vector2 GetSignVector() const;

	FORCEINLINE Vector2 GetAbs() const;

	std::string ToString() const;

	FORCEINLINE void DiagnosticCheckNaN() 
	{

	}

	FORCEINLINE bool ContainsNaN() const
	{
		return (!MMath::IsFinite(x) || !MMath::IsFinite(y));
	}
};

FORCEINLINE Vector2::Vector2()
	: x(0)
	, y(0)
{

}

FORCEINLINE Vector2::Vector2(float inX, float inY)
	: x(inX)
	, y(inY)
{

}

FORCEINLINE Vector2::Vector2(IntPoint inPos)
	: x((float)inPos.x)
	, y((float)inPos.y)
{

}

FORCEINLINE Vector2 operator*(float scale, const Vector2& v)
{
	return v.operator*(scale);
}

FORCEINLINE Vector2 Vector2::operator+(const Vector2& v) const
{
	return Vector2(x + v.x, y + v.y);
}

FORCEINLINE Vector2 Vector2::operator-(const Vector2& v) const
{
	return Vector2(x - v.x, y - v.y);
}

FORCEINLINE Vector2 Vector2::operator*(float scale) const
{
	return Vector2(x * scale, y * scale);
}

FORCEINLINE Vector2 Vector2::operator/(float scale) const
{
	const float rScale = 1.f / scale;
	return Vector2(x * rScale, y * rScale);
}

FORCEINLINE Vector2 Vector2::operator+(float a) const
{
	return Vector2(x + a, y + a);
}

FORCEINLINE Vector2 Vector2::operator-(float a) const
{
	return Vector2(x - a, y - a);
}

FORCEINLINE Vector2 Vector2::operator*(const Vector2& v) const
{
	return Vector2(x * v.x, y * v.y);
}

FORCEINLINE Vector2 Vector2::operator/(const Vector2& v) const
{
	return Vector2(x / v.x, y / v.y);
}

FORCEINLINE float Vector2::operator|(const Vector2& v) const
{
	return x * v.x + y * v.y;
}

FORCEINLINE float Vector2::operator^(const Vector2& v) const
{
	return x * v.y - y * v.x;
}

FORCEINLINE float Vector2::DotProduct(const Vector2& a, const Vector2& b)
{
	return a | b;
}

FORCEINLINE float Vector2::DistSquared(const Vector2 &v1, const Vector2 &v2)
{
	return MMath::Square(v2.x - v1.x) + MMath::Square(v2.y - v1.y);
}

FORCEINLINE float Vector2::Distance(const Vector2& v1, const Vector2& v2)
{
	return MMath::Sqrt(Vector2::DistSquared(v1, v2));
}

FORCEINLINE float Vector2::CrossProduct(const Vector2& a, const Vector2& b)
{
	return a ^ b;
}

FORCEINLINE bool Vector2::operator==(const Vector2& v) const
{
	return x == v.x && y == v.y;
}

FORCEINLINE bool Vector2::operator!=(const Vector2& v) const
{
	return x != v.x || y != v.y;
}

FORCEINLINE bool Vector2::operator<(const Vector2& other) const
{
	return x < other.x && y < other.y;
}

FORCEINLINE bool Vector2::operator>(const Vector2& other) const
{
	return x > other.x && y > other.y;
}

FORCEINLINE bool Vector2::operator<=(const Vector2& other) const
{
	return x <= other.x && y <= other.y;
}

FORCEINLINE bool Vector2::operator>=(const Vector2& other) const
{
	return x >= other.x && y >= other.y;
}

FORCEINLINE bool Vector2::Equals(const Vector2& v, float tolerance) const
{
	return MMath::Abs(x - v.x) <= tolerance && MMath::Abs(y - v.y) <= tolerance;
}

FORCEINLINE Vector2 Vector2::operator-() const
{
	return Vector2(-x, -y);
}

FORCEINLINE Vector2 Vector2::operator+=(const Vector2& v)
{
	x += v.x; y += v.y;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator-=(const Vector2& v)
{
	x -= v.x; y -= v.y;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator*=(float scale)
{
	x *= scale; y *= scale;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator/=(float v)
{
	const float rv = 1.f / v;
	x *= rv; y *= rv;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator*=(const Vector2& v)
{
	x *= v.x; y *= v.y;
	return *this;
}

FORCEINLINE Vector2 Vector2::operator/=(const Vector2& v)
{
	x /= v.x; y /= v.y;
	return *this;
}

FORCEINLINE float& Vector2::operator[](int32 index)
{
	return ((index == 0) ? x : y);
}

FORCEINLINE float Vector2::operator[](int32 index) const
{
	return ((index == 0) ? x : y);
}

FORCEINLINE void Vector2::Set(float inX, float inY)
{
	x = inX;
	y = inY;
}

FORCEINLINE float Vector2::GetMax() const
{
	return MMath::Max(x, y);
}

FORCEINLINE float Vector2::GetAbsMax() const
{
	return MMath::Max(MMath::Abs(x), MMath::Abs(y));
}

FORCEINLINE float Vector2::GetMin() const
{
	return MMath::Min(x, y);
}

FORCEINLINE float Vector2::Size() const
{
	return MMath::Sqrt(x*x + y * y);
}

FORCEINLINE float Vector2::SizeSquared() const
{
	return x * x + y * y;
}

FORCEINLINE Vector2 Vector2::GetRotated(const float angleDeg) const
{
	float s, c;
	MMath::SinCos(&s, &c, MMath::DegreesToRadians(angleDeg));
	return Vector2(c * x - s * y, s * x + c * y);
}

FORCEINLINE Vector2 Vector2::GetSafeNormal(float tolerance) const
{
	const float squareSum = x * x + y * y;
	if (squareSum > tolerance)
	{
		const float scale = MMath::InvSqrt(squareSum);
		return Vector2(x*scale, y*scale);
	}
	return Vector2(0.f, 0.f);
}

FORCEINLINE void Vector2::Normalize(float tolerance)
{
	const float squareSum = x * x + y * y;
	if (squareSum > tolerance)
	{
		const float scale = MMath::InvSqrt(squareSum);
		x *= scale;
		y *= scale;
		return;
	}
	x = 0.0f;
	y = 0.0f;
}

FORCEINLINE void Vector2::ToDirectionAndLength(Vector2 &outDir, float &outLength) const
{
	outLength = Size();
	if (outLength > SMALL_NUMBER)
	{
		float oneOverLength = 1.0f / outLength;
		outDir = Vector2(x*oneOverLength, y*oneOverLength);
	}
	else
	{
		outDir = Vector2::ZeroVector;
	}
}

FORCEINLINE bool Vector2::IsNearlyZero(float tolerance) const
{
	return MMath::Abs(x) <= tolerance && MMath::Abs(y) <= tolerance;
}

FORCEINLINE bool Vector2::IsZero() const
{
	return x == 0.f && y == 0.f;
}

FORCEINLINE float& Vector2::Component(int32 index)
{
	return (&x)[index];
}

FORCEINLINE float Vector2::Component(int32 index) const
{
	return (&x)[index];
}

FORCEINLINE IntPoint Vector2::GetIntPoint() const
{
	return IntPoint(MMath::RoundToInt(x), MMath::RoundToInt(y));
}

FORCEINLINE Vector2 Vector2::RoundToVector() const
{
	return Vector2(MMath::RoundToInt(x), MMath::RoundToInt(y));
}

FORCEINLINE Vector2 Vector2::ClampAxes(float minAxisVal, float maxAxisVal) const
{
	return Vector2(MMath::Clamp(x, minAxisVal, maxAxisVal), MMath::Clamp(y, minAxisVal, maxAxisVal));
}

FORCEINLINE Vector2 Vector2::GetSignVector() const
{
	return Vector2
	(
		MMath::FloatSelect(x, 1.f, -1.f),
		MMath::FloatSelect(y, 1.f, -1.f)
	);
}

FORCEINLINE Vector2 Vector2::GetAbs() const
{
	return Vector2(MMath::Abs(x), MMath::Abs(y));
}

FORCEINLINE std::string Vector2::ToString() const
{
	return StringUtils::Printf("x=%3.3f y=%3.3f", x, y);
}
