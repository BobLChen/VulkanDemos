#pragma once

#include "Configuration/Platform.h"

typedef unsigned char       uint8;
typedef unsigned short int  uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;

typedef signed char         int8;
typedef signed short int    int16;
typedef signed int          int32;
typedef signed long long    int64;

#ifdef _WIN64
typedef unsigned __int64    SIZE_T;
typedef __int64             SSIZE_T;
#else
typedef unsigned long       SIZE_T;
typedef long                SSIZE_T;
#endif

#define ENGINE_NAME "MONKEY"

#if PLATFORM_WINDOWS
    #define FORCE_INLINE __inline
    #define CONSTEXPR constexpr
#else
    #define FORCE_INLINE inline
    #define CONSTEXPR constexpr
#endif
