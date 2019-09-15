#pragma once

#include "Math/GenericPlatformMath.h"
#include "Configuration/Platform.h"
#include <xmmintrin.h>

struct LinuxPlatformMath : public GenericPlatformMath
{
    static FORCEINLINE uint32 CountLeadingZeros(uint32 value)
    {
        if (value == 0)
        {
            return 32;
        }
        return __builtin_clz(value);
    }
    
    static FORCEINLINE uint64 CountLeadingZeros64(uint64 value)
    {
        if (value == 0)
        {
            return 64;
        }
        return __builtin_clzll(value);
    }
    
    static FORCEINLINE uint32 CountTrailingZeros(uint32 value)
    {
        if (value == 0)
        {
            return 32;
        }
        return __builtin_ctz(value);
    }
    
    static FORCEINLINE uint64 CountTrailingZeros64(uint64 value)
    {
        if (value == 0)
        {
            return 64;
        }
        return __builtin_ctzll(value);
    }
    
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
    
    static FORCEINLINE int32 CountBits(uint64 bits)
    {
        return __builtin_popcountll(bits);
    }
    
    static FORCEINLINE float CeilToFloat(float f)
    {
        return (float)CeilToInt(f);
    }
    
    static FORCEINLINE bool IsNaN(float a)
    {
        return isnan(a) != 0;
    }
    
    static FORCEINLINE bool IsFinite(float a)
    {
        return isfinite(a);
    }
};

typedef LinuxPlatformMath PlatformMath;
