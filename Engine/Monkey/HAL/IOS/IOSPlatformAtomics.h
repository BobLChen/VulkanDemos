#pragma once

#include "HAL/ClangPlatformAtomics.h"

struct IOSPlatformAtomics : public ClangPlatformAtomics
{
    
};

typedef IOSPlatformAtomics PlatformAtomics;
