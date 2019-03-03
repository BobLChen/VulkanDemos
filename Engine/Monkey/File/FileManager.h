#pragma once

#include "Configuration/Platform.h"
#include "Common/Common.h"
#include "Common/Log.h"
#include <string>

class FileManager
{
public:
	static bool ReadFile(const std::string& filepath, uint8*& dataPtr, uint32& dataSize);

	static std::string GetFilePath(const std::string& filepath);
};