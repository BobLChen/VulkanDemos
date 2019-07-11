#pragma once

#include "TextureBase.h"

#include <string>
#include <vector>

class Texture2D : public TextureBase
{
public:
	Texture2D();

	virtual ~Texture2D();

	void LoadFromFile(const std::string& filename);

	void LoadFromFiles(const std::vector<std::string>& filenames);
};
