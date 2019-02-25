#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Utils/StringUtils.h"
#include "Math.h"
#include <string>

struct Vector;

struct IntVector
{
public:
	int32 x;
	int32 y;
	int32 z;

public:
	static const IntVector ZeroValue;
	static const IntVector NoneValue;

public:

	IntVector();

	IntVector(int32 inX, int32 inY, int32 inZ);

	explicit IntVector(int32 inValue);

	explicit IntVector(Vector v);

	const int32& operator()(int32 index) const;

	int32& operator()(int32 index);

	const int32& operator[](int32 index) const;

	int32& operator[](int32 index);

	bool operator==(const IntVector& other) const;

	bool operator!=(const IntVector& other) const;

	IntVector& operator*=(int32 scale);

	IntVector& operator/=(int32 divisor);

	IntVector& operator+=(const IntVector& other);

	IntVector& operator-=(const IntVector& other);

	IntVector& operator=(const IntVector& other);

	IntVector operator*(int32 scale) const;

	IntVector operator/(int32 divisor) const;

	IntVector operator+(const IntVector& other) const;

	IntVector operator-(const IntVector& other) const;

	bool IsZero() const;

	float GetMax() const;

	float GetMin() const;

	int32 Size() const;

	std::string ToString() const;

	static IntVector DivideAndRoundUp(IntVector lhs, int32 divisor);

	static int32 Num();
};

FORCEINLINE IntVector::IntVector()
	: x(0)
	, y(0)
	, z(0)
{ 

}

FORCEINLINE IntVector::IntVector(int32 inX, int32 inY, int32 inZ)
	: x(inX)
	, y(inY)
	, z(inZ)
{ 

}

FORCEINLINE IntVector::IntVector(int32 inValue)
	: x(inValue)
	, y(inValue)
	, z(inValue)
{ 

}

FORCEINLINE const int32& IntVector::operator()(int32 index) const
{
	return (&x)[index];
}

FORCEINLINE int32& IntVector::operator()(int32 index)
{
	return (&x)[index];
}

FORCEINLINE const int32& IntVector::operator[](int32 index) const
{
	return (&x)[index];
}

FORCEINLINE int32& IntVector::operator[](int32 index)
{
	return (&x)[index];
}

FORCEINLINE bool IntVector::operator==(const IntVector& other) const
{
	return x == other.x && y == other.y && z == other.z;
}

FORCEINLINE bool IntVector::operator!=(const IntVector& other) const
{
	return x != other.x || y != other.y || z != other.z;
}

FORCEINLINE IntVector& IntVector::operator*=(int32 scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	return *this;
}

FORCEINLINE IntVector& IntVector::operator/=(int32 divisor)
{
	x /= divisor;
	y /= divisor;
	z /= divisor;
	return *this;
}

FORCEINLINE IntVector& IntVector::operator+=(const IntVector& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

FORCEINLINE IntVector& IntVector::operator-=(const IntVector& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

FORCEINLINE IntVector& IntVector::operator=(const IntVector& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}

FORCEINLINE IntVector IntVector::operator*(int32 scale) const
{
	return IntVector(*this) *= scale;
}

FORCEINLINE IntVector IntVector::operator/(int32 divisor) const
{
	return IntVector(*this) /= divisor;
}

FORCEINLINE IntVector IntVector::operator+(const IntVector& other) const
{
	return IntVector(*this) += other;
}

FORCEINLINE IntVector IntVector::operator-(const IntVector& other) const
{
	return IntVector(*this) -= other;
}

FORCEINLINE IntVector IntVector::DivideAndRoundUp(IntVector lhs, int32 divisor)
{
	return IntVector(MMath::DivideAndRoundUp(lhs.x, divisor), MMath::DivideAndRoundUp(lhs.y, divisor), MMath::DivideAndRoundUp(lhs.z, divisor));
}

FORCEINLINE float IntVector::GetMax() const
{
	return MMath::Max(MMath::Max(x, y), z);
}

FORCEINLINE float IntVector::GetMin() const
{
	return MMath::Min(MMath::Min(x, y), z);
}

FORCEINLINE int32 IntVector::Num()
{
	return 3;
}

FORCEINLINE int32 IntVector::Size() const
{
	int64 X64 = (int64)x;
	int64 Y64 = (int64)y;
	int64 Z64 = (int64)z;
	return int32(MMath::Sqrt(float(X64 * X64 + Y64 * Y64 + Z64 * Z64)));
}

FORCEINLINE bool IntVector::IsZero() const
{
	return *this == ZeroValue;
}

FORCEINLINE std::string IntVector::ToString() const
{
	return StringUtils::Printf("x=%d y=%d z=%d", x, y, z);
}

struct IntVector4
{
	int32 x;
	int32 y;
	int32 z;
	int32 w;

	FORCEINLINE IntVector4()
	{

	}

	FORCEINLINE IntVector4(int32 inX, int32 inY, int32 inZ, int32 InW)
		: x(inX)
		, y(inY)
		, z(inZ)
		, w(InW)
	{

	}

	FORCEINLINE explicit IntVector4(int32 inValue)
		: x(inValue)
		, y(inValue)
		, z(inValue)
		, w(inValue)
	{

	}

	FORCEINLINE const int32& operator[](int32 index) const
	{
		return (&x)[index];
	}

	FORCEINLINE int32& operator[](int32 index)
	{
		return (&x)[index];
	}

	FORCEINLINE bool operator==(const IntVector4& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	FORCEINLINE bool operator!=(const IntVector4& other) const
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}
};

struct UintVector4
{
	uint32 x;
	uint32 y;
	uint32 z;
	uint32 w;

	FORCEINLINE UintVector4()
	{

	}

	FORCEINLINE UintVector4(uint32 inX, uint32 inY, uint32 inZ, uint32 InW)
		: x(inX)
		, y(inY)
		, z(inZ)
		, w(InW)
	{

	}

	FORCEINLINE explicit UintVector4(uint32 inValue)
		: x(inValue)
		, y(inValue)
		, z(inValue)
		, w(inValue)
	{

	}

	FORCEINLINE const uint32& operator[](int32 index) const
	{
		return (&x)[index];
	}

	FORCEINLINE uint32& operator[](int32 index)
	{
		return (&x)[index];
	}

	FORCEINLINE bool operator==(const UintVector4& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	FORCEINLINE bool operator!=(const UintVector4& other) const
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}
};
