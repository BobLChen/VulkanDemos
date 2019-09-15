#pragma once

#include "Math/GenericPlatformMath.h"
#include "Configuration/Platform.h"

#include <xmmintrin.h>

struct WindowsPlatformMath : public GenericPlatformMath
{

	static FORCEINLINE int32 TruncToInt(float f)
	{
		return _mm_cvtt_ss2si(_mm_set_ss(f));
	}

	static FORCEINLINE float TruncToFloat(float f)
	{
		return (float)TruncToInt(f);
	}

	static FORCEINLINE int32 RoundToInt(float f)
	{
		return _mm_cvt_ss2si(_mm_set_ss(f + f + 0.5f)) >> 1;
	}

	static FORCEINLINE float RoundToFloat(float f)
	{
		return (float)RoundToInt(f);
	}

	static FORCEINLINE int32 FloorToInt(float f)
	{
		return _mm_cvt_ss2si(_mm_set_ss(f + f - 0.5f)) >> 1;
	}

	static FORCEINLINE float FloorToFloat(float f)
	{
		return (float)FloorToInt(f);
	}

	static FORCEINLINE int32 CeilToInt(float f)
	{
		return -(_mm_cvt_ss2si(_mm_set_ss(-0.5f - (f + f))) >> 1);
	}

	static FORCEINLINE float CeilToFloat(float f)
	{
		return (float)CeilToInt(f);
	}

	static FORCEINLINE bool IsNaN(float f)
	{ 
		return _isnan(f) != 0;
	}

	static FORCEINLINE bool IsFinite(float f)
	{ 
		return _finite(f) != 0;
	}

	static FORCEINLINE float InvSqrt(float f)
	{
		const __m128 fOneHalf = _mm_set_ss(0.5f);
		__m128 y0, x0, x1, x2, fOver2;
		float temp;

		y0 = _mm_set_ss(f);
		x0 = _mm_rsqrt_ss(y0);
		fOver2 = _mm_mul_ss(y0, fOneHalf);

		x1 = _mm_mul_ss(x0, x0);
		x1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(fOver2, x1));
		x1 = _mm_add_ss(x0, _mm_mul_ss(x0, x1));

		x2 = _mm_mul_ss(x1, x1);
		x2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(fOver2, x2));
		x2 = _mm_add_ss(x1, _mm_mul_ss(x1, x2));

		_mm_store_ss(&temp, x2);
		return temp;
	}

	static FORCEINLINE float InvSqrtEst(float f)
	{
		const __m128 fOneHalf = _mm_set_ss(0.5f);
		__m128 y0, x0, x1, fOver2;
		float temp;

		y0 = _mm_set_ss(f);
		x0 = _mm_rsqrt_ss(y0);
		fOver2 = _mm_mul_ss(y0, fOneHalf);

		x1 = _mm_mul_ss(x0, x0);
		x1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(fOver2, x1));
		x1 = _mm_add_ss(x0, _mm_mul_ss(x0, x1));

		_mm_store_ss(&temp, x1);
		return temp;
	}

#pragma intrinsic(_BitScanReverse)
	static FORCEINLINE uint32 FloorLog2(uint32 value)
	{
		unsigned long log2;
		if (_BitScanReverse(&log2, value) != 0)
		{
			return log2;
		}

		return 0;
	}

	static FORCEINLINE uint32 CountLeadingZeros(uint32 value)
	{
		unsigned long log2;
		if (_BitScanReverse(&log2, value) != 0)
		{
			return 31 - log2;
		}

		return 32;
	}

	static FORCEINLINE uint32 CountTrailingZeros(uint32 value)
	{
		if (value == 0)
		{
			return 32;
		}

		unsigned long bitIndex;
		_BitScanForward(&bitIndex, value);
		return bitIndex;
	}

	static FORCEINLINE uint32 CeilLogTwo(uint32 value)
	{
		int32 bitmask = ((int32)(CountLeadingZeros(value) << 26)) >> 31;
		return (32 - CountLeadingZeros(value - 1)) & (~bitmask);
	}

	static FORCEINLINE uint32 RoundUpToPowerOfTwo(uint32 value)
	{
		return 1 << CeilLogTwo(value);
	}

	static FORCEINLINE uint64 RoundUpToPowerOfTwo64(uint64 value)
	{
		return uint64(1) << CeilLogTwo64(value);
	}

#if PLATFORM_64BITS

	static FORCEINLINE uint64 CeilLogTwo64(uint64 value)
	{
		int64 bitmask = ((int64)(CountLeadingZeros64(value) << 57)) >> 63;
		return (64 - CountLeadingZeros64(value - 1)) & (~bitmask);
	}

	static FORCEINLINE uint64 CountLeadingZeros64(uint64 value)
	{
		unsigned long log2;
		if (_BitScanReverse64(&log2, value) != 0)
		{
			return 63 - log2;
		}

		return 64;
	}

	static FORCEINLINE uint64 CountTrailingZeros64(uint64 value)
	{
		if (value == 0)
		{
			return 64;
		}
		unsigned long bitIndex;
		_BitScanForward64(&bitIndex, value);
		return bitIndex;
	}
#endif

};

typedef WindowsPlatformMath PlatformMath;