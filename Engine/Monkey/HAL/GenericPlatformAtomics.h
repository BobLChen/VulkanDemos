#pragma once

#include "Common/Common.h"

struct GenericPlatformAtomics
{
	static FORCEINLINE bool CanUseCompareExchange128()
	{
		return false;
	}
};
