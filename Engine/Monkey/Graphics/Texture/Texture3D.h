#pragma once

#include "TextureBase.h"

#include <string>
#include <vector>

class Texture3D : public TextureBase
{
public:
	Texture3D();

	virtual ~Texture3D();

	void LoadFromBuffer(uint8* data, int32 width, int32 height, int32 depth, VkFormat format);
};
