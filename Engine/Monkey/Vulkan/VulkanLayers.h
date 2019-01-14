#pragma once

#include "VulkanPlatform.h"

#include <vector>

struct VulkanLayerExtension
{
	VulkanLayerExtension();

	void AddUniqueExtensionNames(std::vector<std::string>& outExtensions);

	void AddAnsiExtensionNames(std::vector<const char*>& outExtensions);

	VkLayerProperties layerProps;
	std::vector<VkExtensionProperties> extensionProps;
};
