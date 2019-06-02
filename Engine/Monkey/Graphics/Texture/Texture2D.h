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

	void LoadFromBuffer(void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t width, uint32_t height, VulkanDevice* device, VkQueue copyQueue, VkFilter filter = VK_FILTER_LINEAR, VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

};