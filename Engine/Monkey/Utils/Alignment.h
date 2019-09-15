#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

template <typename T>
FORCEINLINE constexpr T Align(T val, uint64 alignment)
{
    return (T)(((uint64)val + alignment - 1) & ~(alignment - 1));
}

template <typename T>
FORCEINLINE constexpr T AlignDown(T val, uint64 alignment)
{
    return (T)(((uint64)val) & ~(alignment - 1));
}

template <typename T>
FORCEINLINE constexpr bool IsAligned(T val, uint64 alignment)
{
    return !((uint64)val & (alignment - 1));
}

template <typename T>
FORCEINLINE constexpr T AlignArbitrary(T val, uint64 alignment)
{
    return (T)((((uint64)val + alignment - 1) / alignment) * alignment);
}
