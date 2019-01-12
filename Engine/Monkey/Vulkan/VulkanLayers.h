#pragma once

#include "Vulkan/VulkanPlatform.h"

#include <vector>

struct VKLayerExtension
{
	VKLayerExtension()
	{

	}

	void AddUniqueExtensionNames(std::vector<std::string>& outExtensions)
	{
		for (int32 i = 0; i < extensionProps.size(); ++i)
		{
			outExtensions.push_back(extensionProps[i].extensionName);
		}
	}

	VkLayerProperties layerProps;
	std::vector<VkExtensionProperties> extensionProps;
};