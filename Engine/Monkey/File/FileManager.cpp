#include "FileManager.h"
#include "Engine.h"

std::string FileManager::GetFilePath(const std::string& filepath)
{
#if PLATFORM_IOS
    return Engine::Get()->GetAssetsPath() + filepath;
#elif PLATFORM_ANDROID
    
#else
    return Engine::Get()->GetAssetsPath() + "../../../examples/" + filepath;
#endif
}

bool FileManager::ReadFile(const std::string& filepath, uint8*& dataPtr, uint32& dataSize)
{
	std::string finalPath = FileManager::GetFilePath(filepath);

	FILE* file = fopen(finalPath.c_str(), "rb");
	if (!file)
	{
		MLOGE("File not found :%s", filepath.c_str());
		return false;
	}

	fseek(file, 0, SEEK_END);
	dataSize = (uint32)ftell(file);
	fseek(file, 0, SEEK_SET);

	if (dataSize <= 0)
	{
		fclose(file);
		MLOGE("File has no data :%s", filepath.c_str());
		return false;
	}

	dataPtr = new uint8[dataSize];
	fread(dataPtr, 1, dataSize, file);
	fclose(file);

	return true;
}
