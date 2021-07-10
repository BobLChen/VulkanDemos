#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

template <typename T>
FORCE_INLINE constexpr T Align(T val, uint64 alignment)
{
    return (T)(((uint64)val + alignment - 1) & ~(alignment - 1));
}

template <typename T>
FORCE_INLINE constexpr T AlignDown(T val, uint64 alignment)
{
    return (T)(((uint64)val) & ~(alignment - 1));
}

template <typename T>
FORCE_INLINE constexpr bool IsAligned(T val, uint64 alignment)
{
    return !((uint64)val & (alignment - 1));
}

template <typename T>
FORCE_INLINE constexpr T AlignArbitrary(T val, uint64 alignment)
{
    return (T)((((uint64)val + alignment - 1) / alignment) * alignment);
}
