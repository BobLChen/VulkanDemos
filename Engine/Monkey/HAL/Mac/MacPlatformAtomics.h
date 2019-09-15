#pragma once

#include "HAL/ClangPlatformAtomics.h"

struct MacPlatformAtomics : public ClangPlatformAtomics
{
    
};

typedef MacPlatformAtomics PlatformAtomics;
