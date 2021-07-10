#pragma once

#include "IntPoint.h"

#include "Common/Common.h"

struct Vector2
{
public:
	float x;
	float y;

	static const Vector2 ZeroVector;
	static const Vector2 UnitVector;
	
public:

	Vector2();

	Vector2(float inX, float inY);

	Vector2(IntPoint inPos);

	FORCE_INLINE Vector2 operator+(const Vector2& v) const;

	FORCE_INLINE Vector2 operator-(const Vector2& v) const;

	FORCE_INLINE Vector2 operator*(float scale) const;

	FORCE_INLINE Vector2 operator+(float a) const;

	FORCE_INLINE Vector2 operator-(float a) const;

	FORCE_INLINE Vector2 operator*(const Vector2& v) const;

	FORCE_INLINE Vector2 operator/(float scale) const;

	FORCE_INLINE Vector2 operator/(const Vector2& v) const;

	FORCE_INLINE float operator|(const Vector2& v) const;

	FORCE_INLINE float operator^(const Vector2& v) const;

	FORCE_INLINE bool operator==(const Vector2& v) const;

	FORCE_INLINE bool operator!=(const Vector2& v) const;

	FORCE_INLINE bool operator<(const Vector2& other) const;

	FORCE_INLINE bool operator>(const Vector2& other) const;

	FORCE_INLINE bool operator<=(const Vector2& other) const;

	FORCE_INLINE bool operator>=(const Vector2& other) const;

	FORCE_INLINE Vector2 operator-() const;

	FORCE_INLINE Vector2 operator+=(const Vector2& v);

	FORCE_INLINE Vector2 operator-=(const Vector2& v);

	FORCE_INLINE Vector2 operator*=(float scale);

	FORCE_INLINE Vector2 operator/=(float v);

	FORCE_INLINE Vector2 operator*=(const Vector2& v);

	FORCE_INLINE Vector2 operator/=(const Vector2& v);

	FORCE_INLINE float& operator[](int32 index);

	FORCE_INLINE float operator[](int32 index) const;

	FORCE_INLINE float& Component(int32 index);

	FORCE_INLINE float Component(int32 index) const;

	FORCE_INLINE static float DotProduct(const Vector2& a, const Vector2& b);

	FORCE_INLINE static float DistSquared(const Vector2& v1, const Vector2& v2);

	FORCE_INLINE static float Distance(const Vector2& v1, const Vector2& v2);

	FORCE_INLINE static float CrossProduct(const Vector2& a, const Vector2& b);

	FORCE_INLINE bool Equals(const Vector2& v, float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE void Set(float inX, float inY);

	FORCE_INLINE float GetMax() const;

	FORCE_INLINE float GetAbsMax() const;

	FORCE_INLINE float GetMin() const;

	FORCE_INLINE float Size() const;

	FORCE_INLINE float SizeSquared() const;

	FORCE_INLINE Vector2 GetRotated(float angleDeg) const;

	FORCE_INLINE Vector2 GetSafeNormal(float tolerance = SMALL_NUMBER) const;

	FORCE_INLINE void Normalize(float tolerance = SMALL_NUMBER);

	FORCE_INLINE bool IsNearlyZero(float tolerance = KINDA_SMALL_NUMBER) const;

	FORCE_INLINE void ToDirectionAndLength(Vector2 &outDir, float &outLength) const;

	FORCE_INLINE bool IsZero() const;

	FORCE_INLINE IntPoint GetIntPoint() const;

	FORCE_INLINE Vector2 RoundToVector() const;

	FORCE_INLINE Vector2 ClampAxes(float minAxisVal, float maxAxisVal) const;

	FORCE_INLINE Vector2 GetSignVector() const;

	FORCE_INLINE Vector2 GetAbs() const;

	FORCE_INLINE std::string ToString() const;

	FORCE_INLINE void DiagnosticCheckNaN() 
	{
	}

	FORCE_INLINE bool ContainsNaN() const
	{
		return (!MMath::IsFinite(x) || !MMath::IsFinite(y));
	}
};

FORCE_INLINE Vector2::Vector2()
	: x(0)
	, y(0)
{

}

FORCE_INLINE Vector2::Vector2(float inX, float inY)
	: x(inX)
	, y(inY)
{

}

FORCE_INLINE Vector2::Vector2(IntPoint inPos)
	: x((float)inPos.x)
	, y((float)inPos.y)
{

}

FORCE_INLINE Vector2 operator*(float scale, const Vector2& v)
{
	return v.operator*(scale);
}

FORCE_INLINE Vector2 Vector2::operator+(const Vector2& v) const
{
	return Vector2(x + v.x, y + v.y);
}

FORCE_INLINE Vector2 Vector2::operator-(const Vector2& v) const
{
	return Vector2(x - v.x, y - v.y);
}

FORCE_INLINE Vector2 Vector2::operator*(float scale) const
{
	return Vector2(x * scale, y * scale);
}

FORCE_INLINE Vector2 Vector2::operator/(float scale) const
{
	const float invScale = 1.f / scale;
	return Vector2(x * invScale, y * invScale);
}

FORCE_INLINE Vector2 Vector2::operator+(float a) const
{
	return Vector2(x + a, y + a);
}

FORCE_INLINE Vector2 Vector2::operator-(float a) const
{
	return Vector2(x - a, y - a);
}

FORCE_INLINE Vector2 Vector2::operator*(const Vector2& v) const
{
	return Vector2(x * v.x, y * v.y);
}

FORCE_INLINE Vector2 Vector2::operator/(const Vector2& v) const
{
	return Vector2(x / v.x, y / v.y);
}

FORCE_INLINE float Vector2::operator|(const Vector2& v) const
{
	return x * v.x + y * v.y;
}

FORCE_INLINE float Vector2::operator^(const Vector2& v) const
{
	return x * v.y - y * v.x;
}

FORCE_INLINE float Vector2::DotProduct(const Vector2& a, const Vector2& b)
{
	return a | b;
}

FORCE_INLINE float Vector2::DistSquared(const Vector2 &v1, const Vector2 &v2)
{
	return MMath::Square(v2.x - v1.x) + MMath::Square(v2.y - v1.y);
}

FORCE_INLINE float Vector2::Distance(const Vector2& v1, const Vector2& v2)
{
	return MMath::Sqrt(Vector2::DistSquared(v1, v2));
}

FORCE_INLINE float Vector2::CrossProduct(const Vector2& a, const Vector2& b)
{
	return a ^ b;
}

FORCE_INLINE bool Vector2::operator==(const Vector2& v) const
{
	return x == v.x && y == v.y;
}

FORCE_INLINE bool Vector2::operator!=(const Vector2& v) const
{
	return x != v.x || y != v.y;
}

FORCE_INLINE bool Vector2::operator<(const Vector2& other) const
{
	return x < other.x && y < other.y;
}

FORCE_INLINE bool Vector2::operator>(const Vector2& other) const
{
	return x > other.x && y > other.y;
}

FORCE_INLINE bool Vector2::operator<=(const Vector2& other) const
{
	return x <= other.x && y <= other.y;
}

FORCE_INLINE bool Vector2::operator>=(const Vector2& other) const
{
	return x >= other.x && y >= other.y;
}

FORCE_INLINE bool Vector2::Equals(const Vector2& v, float tolerance) const
{
	return MMath::Abs(x - v.x) <= tolerance && MMath::Abs(y - v.y) <= tolerance;
}

FORCE_INLINE Vector2 Vector2::operator-() const
{
	return Vector2(-x, -y);
}

FORCE_INLINE Vector2 Vector2::operator+=(const Vector2& v)
{
	x += v.x; 
	y += v.y;
	return *this;
}

FORCE_INLINE Vector2 Vector2::operator-=(const Vector2& v)
{
	x -= v.x; 
	y -= v.y;
	return *this;
}

FORCE_INLINE Vector2 Vector2::operator*=(float scale)
{
	x *= scale; 
	y *= scale;
	return *this;
}

FORCE_INLINE Vector2 Vector2::operator/=(float v)
{
	const float invF = 1.f / v;
	x *= invF; 
	y *= invF;
	return *this;
}

FORCE_INLINE Vector2 Vector2::operator*=(const Vector2& v)
{
	x *= v.x; 
	y *= v.y;
	return *this;
}

FORCE_INLINE Vector2 Vector2::operator/=(const Vector2& v)
{
	x /= v.x; 
	y /= v.y;
	return *this;
}

FORCE_INLINE float& Vector2::operator[](int32 index)
{
	return ((index == 0) ? x : y);
}

FORCE_INLINE float Vector2::operator[](int32 index) const
{
	return ((index == 0) ? x : y);
}

FORCE_INLINE void Vector2::Set(float inX, float inY)
{
	x = inX;
	y = inY;
}

FORCE_INLINE float Vector2::GetMax() const
{
	return MMath::Max(x, y);
}

FORCE_INLINE float Vector2::GetAbsMax() const
{
	return MMath::Max(MMath::Abs(x), MMath::Abs(y));
}

FORCE_INLINE float Vector2::GetMin() const
{
	return MMath::Min(x, y);
}

FORCE_INLINE float Vector2::Size() const
{
	return MMath::Sqrt(x*x + y * y);
}

FORCE_INLINE float Vector2::SizeSquared() const
{
	return x * x + y * y;
}

FORCE_INLINE Vector2 Vector2::GetRotated(const float angleDeg) const
{
	float s, c;
	MMath::SinCos(&s, &c, MMath::DegreesToRadians(angleDeg));
	return Vector2(c * x - s * y, s * x + c * y);
}

FORCE_INLINE Vector2 Vector2::GetSafeNormal(float tolerance) const
{
	const float squareSum = x * x + y * y;
	if (squareSum > tolerance)
	{
		const float scale = MMath::InvSqrt(squareSum);
		return Vector2(x*scale, y*scale);
	}
	return Vector2(0.f, 0.f);
}

FORCE_INLINE void Vector2::Normalize(float tolerance)
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

FORCE_INLINE void Vector2::ToDirectionAndLength(Vector2 &outDir, float &outLength) const
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

FORCE_INLINE bool Vector2::IsNearlyZero(float tolerance) const
{
	return MMath::Abs(x) <= tolerance && MMath::Abs(y) <= tolerance;
}

FORCE_INLINE bool Vector2::IsZero() const
{
	return x == 0.f && y == 0.f;
}

FORCE_INLINE float& Vector2::Component(int32 index)
{
	return (&x)[index];
}

FORCE_INLINE float Vector2::Component(int32 index) const
{
	return (&x)[index];
}

FORCE_INLINE IntPoint Vector2::GetIntPoint() const
{
	return IntPoint(MMath::RoundToInt(x), MMath::RoundToInt(y));
}

FORCE_INLINE Vector2 Vector2::RoundToVector() const
{
	return Vector2((float)MMath::RoundToInt(x), (float)MMath::RoundToInt(y));
}

FORCE_INLINE Vector2 Vector2::ClampAxes(float minAxisVal, float maxAxisVal) const
{
	return Vector2(MMath::Clamp(x, minAxisVal, maxAxisVal), MMath::Clamp(y, minAxisVal, maxAxisVal));
}

FORCE_INLINE Vector2 Vector2::GetSignVector() const
{
	return Vector2
	(
		MMath::FloatSelect(x, 1.f, -1.f),
		MMath::FloatSelect(y, 1.f, -1.f)
	);
}

FORCE_INLINE Vector2 Vector2::GetAbs() const
{
	return Vector2(MMath::Abs(x), MMath::Abs(y));
}

FORCE_INLINE std::string Vector2::ToString() const
{
	return StringUtils::Printf("x=%3.3f y=%3.3f", x, y);
}
