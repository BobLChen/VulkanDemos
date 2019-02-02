#pragma once

#include "HAL/ClangPlatformAtomics.h"

struct UnixPlatformAtomics : public ClangPlatformAtomics
{
    
};

typedef UnixPlatformAtomics PlatformAtomics;
