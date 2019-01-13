#include "Vulkan/VulkanLayers.h"
#include "Vulkan/VulkanRHI.h"
#include "Utils/StringUtils.h"

static const char* G_ValidationLayersInstance[] =
{
#if PLATFORM_WINDOWS
	"VK_LAYER_LUNARG_standard_validation",
#elif PLATFORM_ANDROID
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
#else
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
#endif
	nullptr
};

static const char* G_ValidationLayersDevice[] =
{
#if PLATFORM_WINDOWS
	"VK_LAYER_LUNARG_standard_validation",
#elif PLATFORM_ANDROID
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
#else
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_image",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_LUNARG_swapchain",
	"VK_LAYER_GOOGLE_unique_objects",
	"VK_LAYER_LUNARG_core_validation",
#endif
	nullptr
};

static const char* G_InstanceExtensions[] =
{
	nullptr
};

static const char* G_DeviceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
	nullptr
};

static inline void EnumerateInstanceExtensionProperties(const char* layerName, VulkanLayerExtension& outLayer)
{
	uint32 count = 0;
	vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateInstanceExtensionProperties(layerName, &count, outLayer.extensionProps.data());
}

static inline void EnumerateDeviceExtensionProperties(VkPhysicalDevice device, const char* layerName, VulkanLayerExtension& outLayer)
{
	uint32 count = 0;
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, outLayer.extensionProps.data());
}

static inline int32 FindLayerIndexInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName)
{
	for (int32 i = 0; i < layers.size(); ++i) 
	{
		if (strcmp(layers[i].layerProps.layerName, layerName) == 0) 
		{
			return i;
		}
	}
	return -1;
}

static inline bool FindLayerInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName)
{
	return FindLayerIndexInList(layers, layerName) != -1;
}

static inline bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName, const char*& foundLayer)
{
	for (int32 i = 0; i < layers.size(); ++i) 
	{
		for (int32 j = 0; j < layers[i].extensionProps.size(); ++j)
		{
			if (strcmp(layers[i].extensionProps[j].extensionName, extensionName) == 0)
			{
				foundLayer = layers[i].layerProps.layerName;
				return true;
			}
		}
	}
	return false;
}

static inline bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName)
{
	const char* dummy = nullptr;
	return FindLayerExtensionInList(layers, extensionName, dummy);
}

static inline void TrimDuplicates(std::vector<const char*>& arr)
{
	for (int32 i = arr.size() - 1; i >= 0; --i)
	{
		bool found = false;
		for (int32 j = i - 1; j >= 0; --j)
		{
			if (strcmp(arr[i], arr[j]) == 0) {
				found = true;
				break;
			}
		}
		if (found) {
			arr.erase(arr.begin() + i);
		}
	}
}

VulkanLayerExtension::VulkanLayerExtension()
{
	memset(&layerProps, 0, sizeof(VkLayerProperties));
}

void VulkanLayerExtension::AddUniqueExtensionNames(std::vector<std::string>& outExtensions)
{
	for (int32 i = 0; i < extensionProps.size(); ++i) 
	{
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
	}
}

void VulkanLayerExtension::AddAnsiExtensionNames(std::vector<const char*>& outExtensions)
{
	for (int32 i = 0; i < extensionProps.size(); ++i) 
	{
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
	}
}

void VulkanRHI::GetInstanceLayersAndExtensions(std::vector<const char*>& outInstanceExtensions, std::vector<const char*>& outInstanceLayers, bool& outDebugUtils)
{
	outDebugUtils = false;

	std::vector<std::string> foundUniqueExtensions;
	std::vector<std::string> foundUniqueLayers;

	std::vector<VulkanLayerExtension> globalLayerExtensions(1);
	EnumerateInstanceExtensionProperties(nullptr, globalLayerExtensions[0]);
	for (int32 i = 0; i < globalLayerExtensions[0].extensionProps.size(); ++i) 
	{
		StringUtils::AddUnique(foundUniqueExtensions, globalLayerExtensions[0].extensionProps[i].extensionName);
	}
	
	uint32 instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	std::vector<VkLayerProperties> globalLayerProperties(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, globalLayerProperties.data());

	for (int32 i = 0; i < globalLayerProperties.size(); ++i) 
	{
		VulkanLayerExtension layer;
		layer.layerProps = globalLayerProperties[i];
		EnumerateInstanceExtensionProperties(globalLayerProperties[i].layerName, layer);
		layer.AddUniqueExtensionNames(foundUniqueExtensions);
		StringUtils::AddUnique(foundUniqueLayers, globalLayerProperties[i].layerName);
		globalLayerExtensions.push_back(layer);
	}

	for (const std::string& name : foundUniqueLayers) 
	{
		MLOG("- Found instance layer %s", name.c_str());
	}

	for (const std::string& name : foundUniqueExtensions) 
	{
		MLOG("- Found instance extension %s", name.c_str());
	}
	
	VulkanPlatform::NotifyFoundInstanceLayersAndExtensions(foundUniqueLayers, foundUniqueExtensions);

#if MONKEY_DEBUG
	for (int32 i = 0; G_ValidationLayersInstance[i] != nullptr; ++i) 
	{
		const char* currValidationLayer = G_ValidationLayersInstance[i];
		bool found = FindLayerInList(globalLayerExtensions, currValidationLayer);
		if (found) 
		{
			outInstanceLayers.push_back(currValidationLayer);
		}
		else 
		{
			MLOG("Unable to find Vulkan instance validation layer '%s'", currValidationLayer);
		}
	}

	const char* foundDebugUtilsLayer = nullptr;
	outDebugUtils = FindLayerExtensionInList(globalLayerExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, foundDebugUtilsLayer);
	if (outDebugUtils && *foundDebugUtilsLayer) 
	{
		outInstanceLayers.push_back(foundDebugUtilsLayer);
	}
#endif // MONKEY_DEBUG
	
	std::vector<const char*> platformExtensions;
	VulkanPlatform::GetInstanceExtensions(platformExtensions);

	for (const char* extension : platformExtensions) 
	{
		if (FindLayerExtensionInList(globalLayerExtensions, extension))
		{
			outInstanceExtensions.push_back(extension);
		}
	}

	for (int32 i = 0; G_InstanceExtensions[i] != nullptr; ++i)
	{
		if (FindLayerExtensionInList(globalLayerExtensions, G_InstanceExtensions[i]))
		{
			outInstanceExtensions.push_back(G_InstanceExtensions[i]);
		}
	}

#if MONKEY_DEBUG
	if (outDebugUtils && FindLayerExtensionInList(globalLayerExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
	{
		outInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
#endif

	TrimDuplicates(outInstanceLayers);
	if (outInstanceLayers.size() > 0)
	{
		MLOG("Using instance layers");
		for (const char* layer : outInstanceLayers)
		{
			MLOG("* %s", layer);
		}
	}
	else
	{
		MLOG("Not using instance layers");
	}

	TrimDuplicates(outInstanceExtensions);
	if (outInstanceExtensions.size() > 0)
	{
		MLOG("Using instance extensions");
		for (const char* extension : outInstanceExtensions)
		{
			MLOG("* %s", extension);
		}
	}
	else
	{
		MLOG("Not using instance extensions");
	}
}