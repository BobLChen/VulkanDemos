#pragma once

#include "Math/GenericPlatformMath.h"
#include "Configuration/Platform.h"
#include <xmmintrin.h>

struct MacPlatformMath : public GenericPlatformMath
{
    static FORCEINLINE uint32 CountLeadingZeros(uint32 Value)
    {
        if (Value == 0)
        {
            return 32;
        }
        return __builtin_clz(Value);
    }
    
    static FORCEINLINE uint64 CountLeadingZeros64(uint64 Value)
    {
        if (Value == 0)
        {
            return 64;
        }
        return __builtin_clzll(Value);
    }
    
    static FORCEINLINE uint32 CountTrailingZeros(uint32 Value)
    {
        if (Value == 0)
        {
            return 32;
        }
        return __builtin_ctz(Value);
    }
    
    static FORCEINLINE uint64 CountTrailingZeros64(uint64 Value)
    {
        if (Value == 0)
        {
            return 64;
        }
        return __builtin_ctzll(Value);
    }
    
    static FORCEINLINE int32 TruncToInt(float F)
    {
        return _mm_cvtt_ss2si(_mm_set_ss(F));
    }
    
    static FORCEINLINE float TruncToFloat(float F)
    {
        return (float)TruncToInt(F);
    }
    
    static FORCEINLINE int32 RoundToInt(float F)
    {
        return _mm_cvt_ss2si(_mm_set_ss(F + F + 0.5f)) >> 1;
    }
    
    static FORCEINLINE float RoundToFloat(float F)
    {
        return (float)RoundToInt(F);
    }
    
    static FORCEINLINE int32 FloorToInt(float F)
    {
        return _mm_cvt_ss2si(_mm_set_ss(F + F - 0.5f)) >> 1;
    }
    
    static FORCEINLINE float FloorToFloat(float F)
    {
        return (float)FloorToInt(F);
    }
    
    static FORCEINLINE int32 CeilToInt(float F)
    {
        return -(_mm_cvt_ss2si(_mm_set_ss(-0.5f - (F + F))) >> 1);
    }
    
    static FORCEINLINE int32 CountBits(uint64 Bits)
    {
        return __builtin_popcountll(Bits);
    }
    
    static FORCEINLINE float CeilToFloat(float F)
    {
        return (float)CeilToInt(F);
    }
    
    static FORCEINLINE bool IsNaN( float A )
    {
        return isnan(A) != 0;
    }
    
    static FORCEINLINE bool IsFinite( float A )
    {
        return isfinite(A);
    }
};

typedef MacPlatformMath PlatformMath;
