#pragma once

#include "Common/Common.h"
#include "Math/IntPoint.h"

struct Vector2D
{
	float x;
	float y;

public:
	static const Vector2D ZeroVector;
	static const Vector2D UnitVector;

public:
	FORCEINLINE Vector2D() 
	{

	}

	FORCEINLINE Vector2D(float inX, float inY);

	FORCEINLINE Vector2D(IntPoint inPos);

public:

	FORCEINLINE Vector2D operator+(const Vector2D& v) const;

	FORCEINLINE Vector2D operator-(const Vector2D& v) const;

	FORCEINLINE Vector2D operator*(float scale) const;

	FORCEINLINE Vector2D operator+(float a) const;

	FORCEINLINE Vector2D operator-(float a) const;

	FORCEINLINE Vector2D operator*(const Vector2D& v) const;

	Vector2D operator/(float scale) const;

	Vector2D operator/(const Vector2D& v) const;

	FORCEINLINE float operator|(const Vector2D& v) const;

	FORCEINLINE float operator^(const Vector2D& v) const;

public:
	bool operator==(const Vector2D& v) const;

	bool operator!=(const Vector2D& v) const;

	bool operator<(const Vector2D& other) const;

	bool operator>(const Vector2D& other) const;

	bool operator<=(const Vector2D& other) const;

	bool operator>=(const Vector2D& other) const;

	FORCEINLINE Vector2D operator-() const;

	FORCEINLINE Vector2D operator+=(const Vector2D& v);

	FORCEINLINE Vector2D operator-=(const Vector2D& v);

	FORCEINLINE Vector2D operator*=(float scale);

	Vector2D operator/=(float v);

	Vector2D operator*=(const Vector2D& v);

	Vector2D operator/=(const Vector2D& v);

	float& operator[](int32 index);

	float operator[](int32 index) const;

	float& Component(int32 index);

	float Component(int32 index) const;

public:
	FORCEINLINE static float DotProduct(const Vector2D& a, const Vector2D& b);

	FORCEINLINE static float DistSquared(const Vector2D& v1, const Vector2D& v2);

	FORCEINLINE static float Distance(const Vector2D& v1, const Vector2D& v2);

	FORCEINLINE static float CrossProduct(const Vector2D& a, const Vector2D& b);

	bool Equals(const Vector2D& v, float tolerance = KINDA_SMALL_NUMBER) const;

	void Set(float inX, float inY);

	float GetMax() const;

	float GetAbsMax() const;

	float GetMin() const;

	float Size() const;

	float SizeSquared() const;

	Vector2D GetRotated(float angleDeg) const;

	Vector2D GetSafeNormal(float tolerance = SMALL_NUMBER) const;

	void Normalize(float tolerance = SMALL_NUMBER);

	bool IsNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const;

	void ToDirectionAndLength(Vector2D &outDir, float &outLength) const;

	bool IsZero() const;

	IntPoint GetIntPoint() const;

	Vector2D RoundToVector() const;

	Vector2D ClampAxes(float minAxisVal, float maxAxisVal) const;

	FORCEINLINE Vector2D GetSignVector() const;

	FORCEINLINE Vector2D GetAbs() const;

	std::string ToString() const;

	FORCEINLINE void DiagnosticCheckNaN() 
	{

	}

	FORCEINLINE bool ContainsNaN() const
	{
		return (!MMath::IsFinite(x) || !MMath::IsFinite(y));
	}
};

FORCEINLINE Vector2D operator*(float scale, const Vector2D& v)
{
	return v.operator*(scale);
}

FORCEINLINE Vector2D::Vector2D(float inX, float inY)
	: x(inX)
	, y(inY)
{

}

FORCEINLINE Vector2D::Vector2D(IntPoint inPos)
	: x((float)inPos.x)
	, y((float)inPos.y)
{

}

FORCEINLINE Vector2D Vector2D::operator+(const Vector2D& v) const
{
	return Vector2D(x + v.x, y + v.y);
}

FORCEINLINE Vector2D Vector2D::operator-(const Vector2D& v) const
{
	return Vector2D(x - v.x, y - v.y);
}

FORCEINLINE Vector2D Vector2D::operator*(float scale) const
{
	return Vector2D(x * scale, y * scale);
}

FORCEINLINE Vector2D Vector2D::operator/(float scale) const
{
	const float rScale = 1.f / scale;
	return Vector2D(x * rScale, y * rScale);
}

FORCEINLINE Vector2D Vector2D::operator+(float a) const
{
	return Vector2D(x + a, y + a);
}

FORCEINLINE Vector2D Vector2D::operator-(float a) const
{
	return Vector2D(x - a, y - a);
}

FORCEINLINE Vector2D Vector2D::operator*(const Vector2D& v) const
{
	return Vector2D(x * v.x, y * v.y);
}

FORCEINLINE Vector2D Vector2D::operator/(const Vector2D& v) const
{
	return Vector2D(x / v.x, y / v.y);
}


FORCEINLINE float Vector2D::operator|(const Vector2D& v) const
{
	return x * v.x + y * v.y;
}

FORCEINLINE float Vector2D::operator^(const Vector2D& v) const
{
	return x * v.y - y * v.x;
}

FORCEINLINE float Vector2D::DotProduct(const Vector2D& a, const Vector2D& b)
{
	return a | b;
}

FORCEINLINE float Vector2D::DistSquared(const Vector2D &v1, const Vector2D &v2)
{
	return MMath::Square(v2.x - v1.x) + MMath::Square(v2.y - v1.y);
}

FORCEINLINE float Vector2D::Distance(const Vector2D& v1, const Vector2D& v2)
{
	return MMath::Sqrt(Vector2D::DistSquared(v1, v2));
}

FORCEINLINE float Vector2D::CrossProduct(const Vector2D& a, const Vector2D& b)
{
	return a ^ b;
}

FORCEINLINE bool Vector2D::operator==(const Vector2D& v) const
{
	return x == v.x && y == v.y;
}

FORCEINLINE bool Vector2D::operator!=(const Vector2D& v) const
{
	return x != v.x || y != v.y;
}

FORCEINLINE bool Vector2D::operator<(const Vector2D& other) const
{
	return x < other.x && y < other.y;
}

FORCEINLINE bool Vector2D::operator>(const Vector2D& other) const
{
	return x > other.x && y > other.y;
}

FORCEINLINE bool Vector2D::operator<=(const Vector2D& other) const
{
	return x <= other.x && y <= other.y;
}

FORCEINLINE bool Vector2D::operator>=(const Vector2D& other) const
{
	return x >= other.x && y >= other.y;
}

FORCEINLINE bool Vector2D::Equals(const Vector2D& v, float tolerance) const
{
	return MMath::Abs(x - v.x) <= tolerance && MMath::Abs(y - v.y) <= tolerance;
}

FORCEINLINE Vector2D Vector2D::operator-() const
{
	return Vector2D(-x, -y);
}

FORCEINLINE Vector2D Vector2D::operator+=(const Vector2D& v)
{
	x += v.x; y += v.y;
	return *this;
}

FORCEINLINE Vector2D Vector2D::operator-=(const Vector2D& v)
{
	x -= v.x; y -= v.y;
	return *this;
}

FORCEINLINE Vector2D Vector2D::operator*=(float scale)
{
	x *= scale; y *= scale;
	return *this;
}

FORCEINLINE Vector2D Vector2D::operator/=(float v)
{
	const float rv = 1.f / v;
	x *= rv; y *= rv;
	return *this;
}

FORCEINLINE Vector2D Vector2D::operator*=(const Vector2D& v)
{
	x *= v.x; y *= v.y;
	return *this;
}

FORCEINLINE Vector2D Vector2D::operator/=(const Vector2D& v)
{
	x /= v.x; y /= v.y;
	return *this;
}

FORCEINLINE float& Vector2D::operator[](int32 index)
{
	return ((index == 0) ? x : y);
}

FORCEINLINE float Vector2D::operator[](int32 index) const
{
	return ((index == 0) ? x : y);
}

FORCEINLINE void Vector2D::Set(float inX, float inY)
{
	x = inX;
	y = inY;
}

FORCEINLINE float Vector2D::GetMax() const
{
	return MMath::Max(x, y);
}

FORCEINLINE float Vector2D::GetAbsMax() const
{
	return MMath::Max(MMath::Abs(x), MMath::Abs(y));
}

FORCEINLINE float Vector2D::GetMin() const
{
	return MMath::Min(x, y);
}


FORCEINLINE float Vector2D::Size() const
{
	return MMath::Sqrt(x*x + y * y);
}

FORCEINLINE float Vector2D::SizeSquared() const
{
	return x * x + y * y;
}

FORCEINLINE Vector2D Vector2D::GetRotated(const float angleDeg) const
{
	float s, c;
	MMath::SinCos(&s, &c, MMath::DegreesToRadians(angleDeg));
	return Vector2D(c * x - s * y, s * x + c * y);
}

FORCEINLINE Vector2D Vector2D::GetSafeNormal(float tolerance) const
{
	const float squareSum = x * x + y * y;
	if (squareSum > tolerance)
	{
		const float scale = MMath::InvSqrt(squareSum);
		return Vector2D(x*scale, y*scale);
	}
	return Vector2D(0.f, 0.f);
}

FORCEINLINE void Vector2D::Normalize(float tolerance)
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

FORCEINLINE void Vector2D::ToDirectionAndLength(Vector2D &outDir, float &outLength) const
{
	outLength = Size();
	if (outLength > SMALL_NUMBER)
	{
		float oneOverLength = 1.0f / outLength;
		outDir = Vector2D(x*oneOverLength, y*oneOverLength);
	}
	else
	{
		outDir = Vector2D::ZeroVector;
	}
}

FORCEINLINE bool Vector2D::IsNearlyZero(float tolerance) const
{
	return MMath::Abs(x) <= tolerance && MMath::Abs(y) <= tolerance;
}

FORCEINLINE bool Vector2D::IsZero() const
{
	return x == 0.f && y == 0.f;
}

FORCEINLINE float& Vector2D::Component(int32 index)
{
	return (&x)[index];
}

FORCEINLINE float Vector2D::Component(int32 index) const
{
	return (&x)[index];
}

FORCEINLINE IntPoint Vector2D::GetIntPoint() const
{
	return IntPoint(MMath::RoundToInt(x), MMath::RoundToInt(y));
}

FORCEINLINE Vector2D Vector2D::RoundToVector() const
{
	return Vector2D(MMath::RoundToInt(x), MMath::RoundToInt(y));
}

FORCEINLINE Vector2D Vector2D::ClampAxes(float minAxisVal, float maxAxisVal) const
{
	return Vector2D(MMath::Clamp(x, minAxisVal, maxAxisVal), MMath::Clamp(y, minAxisVal, maxAxisVal));
}

FORCEINLINE Vector2D Vector2D::GetSignVector() const
{
	return Vector2D
	(
		MMath::FloatSelect(x, 1.f, -1.f),
		MMath::FloatSelect(y, 1.f, -1.f)
	);
}

FORCEINLINE Vector2D Vector2D::GetAbs() const
{
	return Vector2D(MMath::Abs(x), MMath::Abs(y));
}

FORCEINLINE std::string Vector2D::ToString() const
{
	return StringUtils::Printf("x=%3.3f y=%3.3f", x, y);
}
