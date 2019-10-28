#pragma once

#include "Common/Common.h"

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"

class StbImage
{
public:

	static uint8* LoadFromMemory(const uint8* inBuffer, int32 inSize, int32* outWidth, int32* outHeight, int32* outComp, int32 reqComp);

	static float* LoadFloatFromMemory(const uint8* inBuffer, int32 inSize, int32* outWidth, int32* outHeight, int32* outComp, int32 reqComp);

	static void Free(uint8* data);
};