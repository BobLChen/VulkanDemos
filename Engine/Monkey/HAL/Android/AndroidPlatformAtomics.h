#pragma once

#include "HAL/ClangPlatformAtomics.h"

struct AndroidPlatformAtomics : public ClangPlatformAtomics
{
    
};

typedef AndroidPlatformAtomics PlatformAtomics;
