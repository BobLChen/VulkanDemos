#pragma once

#include "Math/GenericPlatformMath.h"
#include "Configuration/Platform.h"
#include <xmmintrin.h>

struct MacPlatformMath : public GenericPlatformMath
{
    static FORCE_INLINE uint32 CountLeadingZeros(uint32 value)
    {
        if (value == 0)
        {
            return 32;
        }
        return __builtin_clz(value);
    }
    
    static FORCE_INLINE uint64 CountLeadingZeros64(uint64 value)
    {
        if (value == 0)
        {
            return 64;
        }
        return __builtin_clzll(value);
    }
    
    static FORCE_INLINE uint32 CountTrailingZeros(uint32 value)
    {
        if (value == 0)
        {
            return 32;
        }
        return __builtin_ctz(value);
    }
    
    static FORCE_INLINE uint64 CountTrailingZeros64(uint64 value)
    {
        if (value == 0)
        {
            return 64;
        }
        return __builtin_ctzll(value);
    }
    
    static FORCE_INLINE int32 TruncToInt(float f)
    {
        return _mm_cvtt_ss2si(_mm_set_ss(f));
    }
    
    static FORCE_INLINE float TruncToFloat(float f)
    {
        return (float)TruncToInt(f);
    }
    
    static FORCE_INLINE int32 RoundToInt(float f)
    {
        return _mm_cvt_ss2si(_mm_set_ss(f + f + 0.5f)) >> 1;
    }
    
    static FORCE_INLINE float RoundToFloat(float f)
    {
        return (float)RoundToInt(f);
    }
    
    static FORCE_INLINE int32 FloorToInt(float f)
    {
        return _mm_cvt_ss2si(_mm_set_ss(f + f - 0.5f)) >> 1;
    }
    
    static FORCE_INLINE float FloorToFloat(float f)
    {
        return (float)FloorToInt(f);
    }
    
    static FORCE_INLINE int32 CeilToInt(float f)
    {
        return -(_mm_cvt_ss2si(_mm_set_ss(-0.5f - (f + f))) >> 1);
    }
    
    static FORCE_INLINE int32 CountBits(uint64 bits)
    {
        return __builtin_popcountll(bits);
    }
    
    static FORCE_INLINE float CeilToFloat(float f)
    {
        return (float)CeilToInt(f);
    }
    
    static FORCE_INLINE bool IsNaN(float a)
    {
        return isnan(a) != 0;
    }
    
    static FORCE_INLINE bool IsFinite(float a)
    {
        return isfinite(a);
    }
};

typedef MacPlatformMath PlatformMath;
