#pragma once

#include "Common/Common.h"

class StbImage
{
public:

	static uint8* LoadFromMemory(const uint8* inBuffer, int32 inSize, int32* outWidth, int32* outHeight, int32* outComp, int32 reqComp);

	static void Free(uint8* data);
};