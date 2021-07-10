#pragma once

#include "Common/Common.h"

#include <math.h>
#include <stdlib.h>
#include <vector>

struct GenericPlatformMath
{

	static CONSTEXPR FORCE_INLINE float TruncToFloat(float f)
	{
		return (float)TruncToInt(f);
	}

	static CONSTEXPR FORCE_INLINE int32 TruncToInt(float f)
	{
		return (int32)(f);
	}

	static FORCE_INLINE int32 FloorToInt(float f)
	{
		return TruncToInt(floorf(f));
	}

	static FORCE_INLINE float FloorToFloat(float f)
	{
		return floorf(f);
	}

	static FORCE_INLINE double FloorToDouble(double f)
	{
		return floor(f);
	}

	static FORCE_INLINE int32 RoundToInt(float f)
	{
		return FloorToInt(f + 0.5f);
	}

	static FORCE_INLINE float RoundToFloat(float f)
	{
		return FloorToFloat(f + 0.5f);
	}

	static FORCE_INLINE double RoundToDouble(double f)
	{
		return FloorToDouble(f + 0.5);
	}

	static FORCE_INLINE int32 CeilToInt(float f)
	{
		return TruncToInt(ceilf(f));
	}

	static FORCE_INLINE float CeilToFloat(float f)
	{
		return ceilf(f);
	}

	static FORCE_INLINE double CeilToDouble(double f)
	{
		return ceil(f);
	}

	static FORCE_INLINE float Fractional(float value)
	{
		return value - TruncToFloat(value);
	}

	static FORCE_INLINE float Frac(float value)
	{
		return value - FloorToFloat(value);
	}

	static FORCE_INLINE float Modf(const float value, float* outIntPart)
	{
		return modff(value, outIntPart);
	}

	static FORCE_INLINE double Modf(const double value, double* outIntPart)
	{
		return modf(value, outIntPart);
	}

	static FORCE_INLINE float Exp(float value)
	{ 
		return expf(value);
	}

	static FORCE_INLINE float Exp2(float value)
	{ 
		return powf(2.f, value);
	}

	static FORCE_INLINE float Loge(float value)
	{ 
		return logf(value);
	}

	static FORCE_INLINE float LogX(float base, float value)
	{ 
		return Loge(value) / Loge(base);
	}

	static FORCE_INLINE float Log2(float value)
	{ 
		return Loge(value) * 1.4426950f;
	}

	static FORCE_INLINE float Fmod(float x, float y)
	{
		if (fabsf(y) <= 1.e-8f) {
			return 0.f;
		}

		const float quotient = TruncToFloat(x / y);
		float intPortion = y * quotient;

		if (fabsf(intPortion) > fabsf(x)) {
			intPortion = x;
		}

		const float result = x - intPortion;
		return result;
	}
	
	static FORCE_INLINE float Sin(float value) 
	{ 
		return sinf(value);
	}

	static FORCE_INLINE float Asin(float value)
	{ 
		return asinf((value < -1.f) ? -1.f : ((value < 1.f) ? value : 1.f));
	}

	static FORCE_INLINE float Sinh(float value)
	{ 
		return sinhf(value);
	}

	static FORCE_INLINE float Cos(float value)
	{ 
		return cosf(value);
	}

	static FORCE_INLINE float Acos(float value)
	{ 
		return acosf((value < -1.f) ? -1.f : ((value < 1.f) ? value : 1.f));
	}

	static FORCE_INLINE float Tan(float value)
	{ 
		return tanf(value);
	}

	static FORCE_INLINE float Atan(float value)
	{ 
		return atanf(value);
	}

	static FORCE_INLINE float Sqrt(float value)
	{ 
		return sqrtf(value);
	}

	static FORCE_INLINE float Pow(float a, float b) 
	{ 
		return powf(a, b);
	}

	static FORCE_INLINE float InvSqrt(float f)
	{
		return 1.0f / sqrtf(f);
	}

	static FORCE_INLINE float InvSqrtEst(float f)
	{
		return InvSqrt(f);
	}

	static FORCE_INLINE bool IsNaN(float f)
	{
		return ((*(uint32*)&f) & 0x7FFFFFFF) > 0x7F800000;
	}

	static FORCE_INLINE bool IsFinite(float f)
	{
		return ((*(uint32*)&f) & 0x7F800000) != 0x7F800000;
	}

	static FORCE_INLINE bool IsNegativeFloat(const float& f)
	{
		return ((*(uint32*)&f) >= (uint32)0x80000000);
	}

	static FORCE_INLINE bool IsNegativeDouble(const double& f)
	{
		return ((*(uint64*)&f) >= (uint64)0x8000000000000000);
	}

	static FORCE_INLINE int32 Rand() 
	{ 
		return rand();
	}

	static FORCE_INLINE void RandInit(int32 seed) 
	{ 
		srand(seed);
	}

	static FORCE_INLINE float FRand() 
	{ 
		return Rand() / (float)RAND_MAX;
	}

	static FORCE_INLINE uint32 FloorLog2(uint32 value)
	{
		uint32 pos = 0;

		if (value >= 1 << 16) 
		{ 
			value >>= 16; 
			pos += 16;
		}

		if (value >= 1 << 8) 
		{ 
			value >>= 8; 
			pos += 8;
		}

		if (value >= 1 << 4) 
		{ 
			value >>= 4; 
			pos += 4;
		}

		if (value >= 1 << 2) 
		{ 
			value >>= 2; 
			pos += 2;
		}

		if (value >= 1 << 1) 
		{ 
			pos += 1;
		}

		return (value == 0) ? 0 : pos;
	}

	static FORCE_INLINE uint64 FloorLog2_64(uint64 value)
	{
		uint64 pos = 0;
		if (value >= 1ull << 32) 
		{ 
			value >>= 32; 
			pos += 32;
		}

		if (value >= 1ull << 16) 
		{ 
			value >>= 16; 
			pos += 16;
		}

		if (value >= 1ull << 8) 
		{ 
			value >>= 8; 
			pos += 8;
		}

		if (value >= 1ull << 4) 
		{ 
			value >>= 4;
			pos += 4;
		}

		if (value >= 1ull << 2) 
		{ 
			value >>= 2; 
			pos += 2;
		}

		if (value >= 1ull << 1) 
		{ 
			pos += 1;
		}

		return (value == 0) ? 0 : pos;
	}

	static FORCE_INLINE uint32 CountLeadingZeros(uint32 value)
	{
		if (value == 0) {
			return 32;
		}
		return 31 - FloorLog2(value);
	}

	static FORCE_INLINE uint64 CountLeadingZeros64(uint64 value)
	{
		if (value == 0) {
			return 64;
		}
		return 63 - FloorLog2_64(value);
	}

	static FORCE_INLINE uint32 CountTrailingZeros(uint32 value)
	{
		if (value == 0) {
			return 32;
		}

		uint32 result = 0;

		while ((value & 1) == 0) 
		{
			value >>= 1;
			++result;
		}

		return result;
	}

	static FORCE_INLINE uint64 CountTrailingZeros64(uint64 value)
	{
		if (value == 0) {
			return 64;
		}

		uint64 result = 0;
		while ((value & 1) == 0) 
		{
			value >>= 1;
			++result;
		}

		return result;
	}

	static FORCE_INLINE uint32 CeilLogTwo(uint32 value)
	{
		int32 bitmask = ((int32)(CountLeadingZeros(value) << 26)) >> 31;
		return (32 - CountLeadingZeros(value - 1)) & (~bitmask);
	}

	static FORCE_INLINE uint64 CeilLogTwo64(uint64 value)
	{
		int64 bitmask = ((int64)(CountLeadingZeros64(value) << 57)) >> 63;
		return (64 - CountLeadingZeros64(value - 1)) & (~bitmask);
	}

	static FORCE_INLINE uint32 RoundUpToPowerOfTwo(uint32 value)
	{
		return 1 << CeilLogTwo(value);
	}

	static FORCE_INLINE uint64 RoundUpToPowerOfTwo64(uint64 value)
	{
		return uint64(1) << CeilLogTwo64(value);
	}

	static FORCE_INLINE uint32 MortonCode2(uint32 x)
	{
		x &= 0x0000ffff;
		x = (x ^ (x << 8)) & 0x00ff00ff;
		x = (x ^ (x << 4)) & 0x0f0f0f0f;
		x = (x ^ (x << 2)) & 0x33333333;
		x = (x ^ (x << 1)) & 0x55555555;
		return x;
	}

	static FORCE_INLINE uint32 ReverseMortonCode2(uint32 x)
	{
		x &= 0x55555555;
		x = (x ^ (x >> 1)) & 0x33333333;
		x = (x ^ (x >> 2)) & 0x0f0f0f0f;
		x = (x ^ (x >> 4)) & 0x00ff00ff;
		x = (x ^ (x >> 8)) & 0x0000ffff;
		return x;
	}

	static FORCE_INLINE uint32 MortonCode3(uint32 x)
	{
		x &= 0x000003ff;
		x = (x ^ (x << 16)) & 0xff0000ff;
		x = (x ^ (x << 8)) & 0x0300f00f;
		x = (x ^ (x << 4)) & 0x030c30c3;
		x = (x ^ (x << 2)) & 0x09249249;
		return x;
	}

	static FORCE_INLINE uint32 ReverseMortonCode3(uint32 x)
	{
		x &= 0x09249249;
		x = (x ^ (x >> 2)) & 0x030c30c3;
		x = (x ^ (x >> 4)) & 0x0300f00f;
		x = (x ^ (x >> 8)) & 0xff0000ff;
		x = (x ^ (x >> 16)) & 0x000003ff;
		return x;
	}

	static CONSTEXPR FORCE_INLINE float FloatSelect(float comparand, float valueGEZero, float valueLTZero)
	{
		return comparand >= 0.f ? valueGEZero : valueLTZero;
	}

	static CONSTEXPR FORCE_INLINE double FloatSelect(double comparand, double valueGEZero, double valueLTZero)
	{
		return comparand >= 0.f ? valueGEZero : valueLTZero;
	}

	template< class T >
	static CONSTEXPR FORCE_INLINE T Abs(const T a)
	{
		return (a >= (T)0) ? a : -a;
	}

	template< class T >
	static CONSTEXPR FORCE_INLINE T Sign(const T a)
	{
		return (a > (T)0) ? (T)1 : ((a < (T)0) ? (T)-1 : (T)0);
	}

	template< class T >
	static CONSTEXPR FORCE_INLINE T Max(const T a, const T b)
	{
		return (a >= b) ? a : b;
	}

	template< class T >
	static CONSTEXPR FORCE_INLINE T Min(const T a, const T b)
	{
		return (a <= b) ? a : b;
	}

	static FORCE_INLINE int32 CountBits(uint64 bits)
	{
		bits -= (bits >> 1) & 0x5555555555555555ull;
		bits = (bits & 0x3333333333333333ull) + ((bits >> 2) & 0x3333333333333333ull);
		bits = (bits + (bits >> 4)) & 0x0f0f0f0f0f0f0f0full;
		return (bits * 0x0101010101010101) >> 56;
	}

	template< class T >
	static FORCE_INLINE T Min(const std::vector<T>& values, int32* minIndex = NULL)
	{
		if (values.size() == 0)
		{
			if (minIndex) {
				*minIndex = -1;
			}
			return T();
		}

		T curMin = values[0];
		int32 curMinIndex = 0;
		for (int32 v = 1; v < values.size(); ++v)
		{
			const T value = values[v];
			if (value < curMin) 
			{
				curMin = value;
				curMinIndex = v;
			}
		}

		if (minIndex) {
			*minIndex = curMinIndex;
		}

		return curMin;
	}

	template< class T >
	static FORCE_INLINE T Max(const std::vector<T>& values, int32* maxIndex = NULL)
	{
		if (values.size() == 0)
		{
			if (maxIndex) {
				*maxIndex = -1;
			}
			return T();
		}

		T curMax = values[0];
		int32 curMaxIndex = 0;
		for (int32 v = 1; v < values.size(); ++v)
		{
			const T value = values[v];
			if (curMax < value) 
			{
				curMax = value;
				curMaxIndex = v;
			}
		}

		if (maxIndex) {
			*maxIndex = curMaxIndex;
		}

		return curMax;
	}

	static void SRandInit(int32 seed);

	static int32 GetRandSeed();

	static float SRand();

	static float Atan2(float y, float x);
};

template<>
FORCE_INLINE float GenericPlatformMath::Abs(const float a)
{
	return fabsf(a);
}
