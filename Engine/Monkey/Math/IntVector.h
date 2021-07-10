#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Utils/StringUtils.h"
#include "Math.h"
#include <string>

struct Vector3;

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

	explicit IntVector(Vector3 v);

	FORCE_INLINE const int32& operator()(int32 index) const;

	FORCE_INLINE int32& operator()(int32 index);

	FORCE_INLINE const int32& operator[](int32 index) const;

	FORCE_INLINE int32& operator[](int32 index);

	FORCE_INLINE bool operator==(const IntVector& other) const;

	FORCE_INLINE bool operator!=(const IntVector& other) const;

	FORCE_INLINE IntVector& operator*=(int32 scale);

	FORCE_INLINE IntVector& operator/=(int32 divisor);

	FORCE_INLINE IntVector& operator+=(const IntVector& other);

	FORCE_INLINE IntVector& operator-=(const IntVector& other);

	FORCE_INLINE IntVector& operator=(const IntVector& other);

	FORCE_INLINE IntVector operator*(int32 scale) const;

	FORCE_INLINE IntVector operator/(int32 divisor) const;

	FORCE_INLINE IntVector operator+(const IntVector& other) const;

	FORCE_INLINE IntVector operator-(const IntVector& other) const;

	FORCE_INLINE bool IsZero() const;

	FORCE_INLINE int32 GetMax() const;

	FORCE_INLINE int32 GetMin() const;

	FORCE_INLINE int32 Size() const;

	FORCE_INLINE std::string ToString() const;

	FORCE_INLINE static IntVector DivideAndRoundUp(IntVector lhs, int32 divisor);

	FORCE_INLINE static int32 Num();
};

FORCE_INLINE IntVector::IntVector()
	: x(0)
	, y(0)
	, z(0)
{ 

}

FORCE_INLINE IntVector::IntVector(int32 inX, int32 inY, int32 inZ)
	: x(inX)
	, y(inY)
	, z(inZ)
{ 

}

FORCE_INLINE IntVector::IntVector(int32 inValue)
	: x(inValue)
	, y(inValue)
	, z(inValue)
{ 

}

FORCE_INLINE const int32& IntVector::operator()(int32 index) const
{
	return (&x)[index];
}

FORCE_INLINE int32& IntVector::operator()(int32 index)
{
	return (&x)[index];
}

FORCE_INLINE const int32& IntVector::operator[](int32 index) const
{
	return (&x)[index];
}

FORCE_INLINE int32& IntVector::operator[](int32 index)
{
	return (&x)[index];
}

FORCE_INLINE bool IntVector::operator==(const IntVector& other) const
{
	return x == other.x && y == other.y && z == other.z;
}

FORCE_INLINE bool IntVector::operator!=(const IntVector& other) const
{
	return x != other.x || y != other.y || z != other.z;
}

FORCE_INLINE IntVector& IntVector::operator*=(int32 scale)
{
	x *= scale;
	y *= scale;
	z *= scale;
	return *this;
}

FORCE_INLINE IntVector& IntVector::operator/=(int32 divisor)
{
	x /= divisor;
	y /= divisor;
	z /= divisor;
	return *this;
}

FORCE_INLINE IntVector& IntVector::operator+=(const IntVector& other)
{
	x += other.x;
	y += other.y;
	z += other.z;
	return *this;
}

FORCE_INLINE IntVector& IntVector::operator-=(const IntVector& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;
	return *this;
}

FORCE_INLINE IntVector& IntVector::operator=(const IntVector& other)
{
	x = other.x;
	y = other.y;
	z = other.z;
	return *this;
}

FORCE_INLINE IntVector IntVector::operator*(int32 scale) const
{
	return IntVector(*this) *= scale;
}

FORCE_INLINE IntVector IntVector::operator/(int32 divisor) const
{
	return IntVector(*this) /= divisor;
}

FORCE_INLINE IntVector IntVector::operator+(const IntVector& other) const
{
	return IntVector(*this) += other;
}

FORCE_INLINE IntVector IntVector::operator-(const IntVector& other) const
{
	return IntVector(*this) -= other;
}

FORCE_INLINE IntVector IntVector::DivideAndRoundUp(IntVector lhs, int32 divisor)
{
	return IntVector(MMath::DivideAndRoundUp(lhs.x, divisor), MMath::DivideAndRoundUp(lhs.y, divisor), MMath::DivideAndRoundUp(lhs.z, divisor));
}

FORCE_INLINE int32 IntVector::GetMax() const
{
	return MMath::Max<int32>(MMath::Max<int32>(x, y), z);
}

FORCE_INLINE int32 IntVector::GetMin() const
{
	return MMath::Min<int32>(MMath::Min<int32>(x, y), z);
}

FORCE_INLINE int32 IntVector::Num()
{
	return 3;
}

FORCE_INLINE int32 IntVector::Size() const
{
	float xx = (float)(x * x);
	float yy = (float)(y * y);
	float zz = (float)(z * z);
	return int32(MMath::Sqrt(xx + yy + zz));
}

FORCE_INLINE bool IntVector::IsZero() const
{
	return *this == ZeroValue;
}

FORCE_INLINE std::string IntVector::ToString() const
{
	return StringUtils::Printf("x=%d y=%d z=%d", x, y, z);
}

struct IntVector4
{
public:
	int32 x;
	int32 y;
	int32 z;
	int32 w;

public:
	FORCE_INLINE IntVector4()
	{

	}

	FORCE_INLINE IntVector4(int32 inX, int32 inY, int32 inZ, int32 InW)
		: x(inX)
		, y(inY)
		, z(inZ)
		, w(InW)
	{

	}

	FORCE_INLINE explicit IntVector4(int32 inValue)
		: x(inValue)
		, y(inValue)
		, z(inValue)
		, w(inValue)
	{

	}

	FORCE_INLINE const int32& operator[](int32 index) const
	{
		return (&x)[index];
	}

	FORCE_INLINE int32& operator[](int32 index)
	{
		return (&x)[index];
	}

	FORCE_INLINE bool operator==(const IntVector4& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	FORCE_INLINE bool operator!=(const IntVector4& other) const
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}
};

struct UintVector4
{
public:
	uint32 x;
	uint32 y;
	uint32 z;
	uint32 w;

public:
	FORCE_INLINE UintVector4()
	{

	}

	FORCE_INLINE UintVector4(uint32 inX, uint32 inY, uint32 inZ, uint32 InW)
		: x(inX)
		, y(inY)
		, z(inZ)
		, w(InW)
	{

	}

	FORCE_INLINE explicit UintVector4(uint32 inValue)
		: x(inValue)
		, y(inValue)
		, z(inValue)
		, w(inValue)
	{

	}

	FORCE_INLINE const uint32& operator[](int32 index) const
	{
		return (&x)[index];
	}

	FORCE_INLINE uint32& operator[](int32 index)
	{
		return (&x)[index];
	}

	FORCE_INLINE bool operator==(const UintVector4& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	FORCE_INLINE bool operator!=(const UintVector4& other) const
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}
};
