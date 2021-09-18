#include "Common/Log.h"

#include "Engine.h"
#include "FileManager.h"

#if PLATFORM_WINDOWS

#elif PLATFORM_MAC

#elif PLATFORM_IOS

#elif PLATFORM_LINUX

#elif PLATFORM_ANDROID
    #include "Application/Android/AndroidWindow.h"
#endif

std::string FileManager::GetFilePath(const std::string& filepath)
{
#if defined (DEMO_RES_PATH)
    return DEMO_RES_PATH + filepath;
#else
    return Engine::Get()->GetAppPath() + filepath;
#endif
}

bool FileManager::ReadFile(const std::string& filepath, uint8*& dataPtr, uint32& dataSize)
{
    std::string finalPath = FileManager::GetFilePath(filepath);

#if PLATFORM_ANDROID

    AAsset* asset = AAssetManager_open(g_AndroidApp->activity->assetManager, finalPath.c_str(), AASSET_MODE_STREAMING);
    dataSize = AAsset_getLength(asset);
    dataPtr = new uint8[dataSize];
    AAsset_read(asset, dataPtr, dataSize);
    AAsset_close(asset);

#else

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

#endif

    return true;
}
