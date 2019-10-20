#include "ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

 #define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

uint8* StbImage::LoadFromMemory(const uint8* inBuffer, int32 inSize, int32* outWidth, int32* outHeight, int32* outComp, int32 reqComp)
{
	return stbi_load_from_memory(inBuffer, inSize, outWidth, outHeight, outComp, reqComp);
}

float* StbImage::LoadFloatFromMemory(const uint8* inBuffer, int32 inSize, int32* outWidth, int32* outHeight, int32* outComp, int32 reqComp)
{
	return stbi_loadf_from_memory(inBuffer, inSize, outWidth, outHeight, outComp, reqComp);
}

void StbImage::Free(uint8* data)
{
	stbi_image_free(data);
}