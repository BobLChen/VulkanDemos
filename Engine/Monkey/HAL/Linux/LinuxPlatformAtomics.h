#pragma once

#include "HAL/ClangPlatformAtomics.h"

struct LinuxPlatformAtomics : public ClangPlatformAtomics
{
    
};

typedef LinuxPlatformAtomics PlatformAtomics;
