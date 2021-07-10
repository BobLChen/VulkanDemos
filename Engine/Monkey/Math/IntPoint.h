#pragma once

#include "Common/Common.h"
#include "Math/Math.h"
#include "Utils/StringUtils.h"

#include <string>
#include <algorithm>
#include <cmath>

struct IntPoint
{
public:

	int32 x;
	int32 y;

	static const IntPoint ZeroValue;
	static const IntPoint NoneValue;

public:
    
	IntPoint();

	IntPoint(int32 inX, int32 inY);

	FORCE_INLINE const int32& operator()(int32 pointIndex) const;

	FORCE_INLINE int32& operator()(int32 pointIndex);

	FORCE_INLINE int32& operator[](int32 index);

	FORCE_INLINE int32 operator[](int32 index) const;

	FORCE_INLINE IntPoint& operator=(const IntPoint& other);

	FORCE_INLINE bool operator==(const IntPoint& other) const;

	FORCE_INLINE bool operator!=(const IntPoint& other) const;

	FORCE_INLINE IntPoint& operator*=(int32 scale);

	FORCE_INLINE IntPoint& operator/=(int32 divisor);

	FORCE_INLINE IntPoint& operator+=(const IntPoint& other);

	FORCE_INLINE IntPoint& operator-=(const IntPoint& other);

	FORCE_INLINE IntPoint& operator/=(const IntPoint& other);

	FORCE_INLINE IntPoint operator*(int32 scale) const;

	FORCE_INLINE IntPoint operator/(int32 divisor) const;

	FORCE_INLINE IntPoint operator+(const IntPoint& other) const;

	FORCE_INLINE IntPoint operator-(const IntPoint& other) const;

	FORCE_INLINE IntPoint operator/(const IntPoint& other) const;

	FORCE_INLINE IntPoint ComponentMin(const IntPoint& other) const;

	FORCE_INLINE IntPoint ComponentMax(const IntPoint& other) const;

	FORCE_INLINE int32 GetMax() const;

	FORCE_INLINE int32 GetMin() const;

	FORCE_INLINE int32 Size() const;

	FORCE_INLINE int32 SizeSquared() const;

	FORCE_INLINE std::string ToString() const;

	static FORCE_INLINE int32 Num();

	static FORCE_INLINE IntPoint DivideAndRoundUp(IntPoint lhs, int32 divisor);

	static FORCE_INLINE IntPoint DivideAndRoundUp(IntPoint lhs, IntPoint divisor);

	static FORCE_INLINE IntPoint DivideAndRoundDown(IntPoint lhs, int32 divisor);
};

FORCE_INLINE IntPoint::IntPoint()
	: x(0)
	, y(0)
{

}

FORCE_INLINE IntPoint::IntPoint(int32 inX, int32 inY)
	: x(inX)
	, y(inY)
{

}

FORCE_INLINE const int32& IntPoint::operator()(int32 pointIndex) const
{
	return (&x)[pointIndex];
}

FORCE_INLINE int32& IntPoint::operator()(int32 pointIndex)
{
	return (&x)[pointIndex];
}

FORCE_INLINE int32 IntPoint::Num()
{
	return 2;
}

FORCE_INLINE bool IntPoint::operator==(const IntPoint& other) const
{
	return x == other.x && y == other.y;
}

FORCE_INLINE bool IntPoint::operator!=(const IntPoint& other) const
{
	return x != other.x || y != other.y;
}

FORCE_INLINE IntPoint& IntPoint::operator*=(int32 scale)
{
	x *= scale;
	y *= scale;
	return *this;
}

FORCE_INLINE IntPoint& IntPoint::operator/=(int32 divisor)
{
	x /= divisor;
	y /= divisor;
	return *this;
}

FORCE_INLINE IntPoint& IntPoint::operator+=(const IntPoint& other)
{
	x += other.x;
	y += other.y;
	return *this;
}

FORCE_INLINE IntPoint& IntPoint::operator-=(const IntPoint& other)
{
	x -= other.x;
	y -= other.y;
	return *this;
}

FORCE_INLINE IntPoint& IntPoint::operator/=(const IntPoint& other)
{
	x /= other.x;
	y /= other.y;
	return *this;
}

FORCE_INLINE IntPoint& IntPoint::operator=(const IntPoint& other)
{
	x = other.x;
	y = other.y;
	return *this;
}

FORCE_INLINE IntPoint IntPoint::operator*(int32 scale) const
{
	return IntPoint(*this) *= scale;
}

FORCE_INLINE IntPoint IntPoint::operator/(int32 divisor) const
{
	return IntPoint(*this) /= divisor;
}

FORCE_INLINE int32& IntPoint::operator[](int32 index)
{
	return ((index == 0) ? x : y);
}

FORCE_INLINE int32 IntPoint::operator[](int32 index) const
{
	return ((index == 0) ? x : y);
}

FORCE_INLINE IntPoint IntPoint::ComponentMin(const IntPoint& other) const
{
	return IntPoint(std::min(x, other.x), std::min(y, other.y));
}

FORCE_INLINE IntPoint IntPoint::ComponentMax(const IntPoint& other) const
{
	return IntPoint(std::max(x, other.x), std::max(y, other.y));
}

FORCE_INLINE IntPoint IntPoint::DivideAndRoundUp(IntPoint lhs, int32 divisor)
{
	return IntPoint(MMath::DivideAndRoundUp(lhs.x, divisor), MMath::DivideAndRoundUp(lhs.y, divisor));
}

FORCE_INLINE IntPoint IntPoint::DivideAndRoundUp(IntPoint lhs, IntPoint divisor)
{
	return IntPoint(MMath::DivideAndRoundUp(lhs.x, divisor.x), MMath::DivideAndRoundUp(lhs.y, divisor.y));
}

FORCE_INLINE IntPoint IntPoint::DivideAndRoundDown(IntPoint lhs, int32 divisor)
{
	return IntPoint(MMath::DivideAndRoundDown(lhs.x, divisor), MMath::DivideAndRoundDown(lhs.y, divisor));
}

FORCE_INLINE IntPoint IntPoint::operator+(const IntPoint& other) const
{
	return IntPoint(*this) += other;
}

FORCE_INLINE IntPoint IntPoint::operator-(const IntPoint& other) const
{
	return IntPoint(*this) -= other;
}

FORCE_INLINE IntPoint IntPoint::operator/(const IntPoint& other) const
{
	return IntPoint(*this) /= other;
}

FORCE_INLINE int32 IntPoint::GetMax() const
{
	return std::max(x, y);
}

FORCE_INLINE int32 IntPoint::GetMin() const
{
	return std::min(x, y);
}

FORCE_INLINE int32 IntPoint::Size() const
{
	int64 x64 = (int64)x;
	int64 y64 = (int64)x;
	return int32(std::sqrt(float(x64 * x64 + y64 * y64)));
}

FORCE_INLINE int32 IntPoint::SizeSquared() const
{
	return x * x + y * y;
}

FORCE_INLINE std::string IntPoint::ToString() const
{
	return StringUtils::Printf("X=%d Y=%d", x, y);
}
