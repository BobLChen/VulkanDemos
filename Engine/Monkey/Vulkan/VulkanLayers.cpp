#include "VulkanPlatform.h"
#include "VulkanRHI.h"
#include "VulkanDevice.h"
#include "Utils/StringUtils.h"

#include <vector>

struct VulkanLayerExtension
{
	VulkanLayerExtension();
	void AddUniqueExtensionNames(std::vector<std::string>& outExtensions);
	void AddUniqueExtensionNames(std::vector<const char*>& outExtensions);

	VkLayerProperties layerProps;
	std::vector<VkExtensionProperties> extensionProps;
};

static const char* G_ValidationLayersInstance[] =
{
#if PLATFORM_WINDOWS
	"VK_LAYER_KHRONOS_validation",
#elif PLATFORM_MAC
	"VK_LAYER_LUNARG_standard_validation",
    "VK_LAYER_GOOGLE_unique_objects",
    "VK_LAYER_GOOGLE_threading",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_parameter_validation",
    "VK_LAYER_LUNARG_object_tracker",
#elif PLATFORM_IOS
    "MoltenVK",
#elif PLATFORM_ANDROID
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_LUNARG_swapchain",
	"VK_LAYER_GOOGLE_unique_objects",
#elif PLATFORM_LINUX
	"VK_LAYER_KHRONOS_validation",
#endif
	nullptr
};

static const char* G_ValidationLayersDevice[] =
{
#if PLATFORM_WINDOWS
	"VK_LAYER_KHRONOS_validation",
#elif PLATFORM_IOS
    "MoltenVK",
#elif PLATFORM_MAC
    "VK_LAYER_LUNARG_standard_validation",
    "VK_LAYER_GOOGLE_unique_objects",
    "VK_LAYER_GOOGLE_threading",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_parameter_validation",
    "VK_LAYER_LUNARG_object_tracker",
#elif PLATFORM_ANDROID
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
#elif PLATFORM_LINUX
	"VK_LAYER_KHRONOS_validation",
#endif
	nullptr
};

static const char* G_InstanceExtensions[] =
{
#if PLATFORM_WINDOWS
	
#elif PLATFORM_MAC

#elif PLATFORM_IOS
    
#elif PLATFORM_LINUX

#elif PLATFORM_ANDROID
    
#endif
	nullptr
};

static const char* G_DeviceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
	VK_KHR_MAINTENANCE1_EXTENSION_NAME,

#if PLATFORM_WINDOWS

#elif PLATFORM_MAC

#elif PLATFORM_IOS

#elif PLATFORM_LINUX

#elif PLATFORM_ANDROID
	
#endif

	nullptr
};

static FORCE_INLINE void EnumerateInstanceExtensionProperties(const char* layerName, VulkanLayerExtension& outLayer)
{
	uint32 count = 0;
	vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateInstanceExtensionProperties(layerName, &count, outLayer.extensionProps.data());
}

static FORCE_INLINE void EnumerateDeviceExtensionProperties(VkPhysicalDevice device, const char* layerName, VulkanLayerExtension& outLayer)
{
	uint32 count = 0;
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, outLayer.extensionProps.data());
}

static FORCE_INLINE int32 FindLayerIndexInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName)
{
	for (int32 i = 0; i < layers.size(); ++i) 
	{
		if (strcmp(layers[i].layerProps.layerName, layerName) == 0) {
			return i;
		}
	}
	return -1;
}

static FORCE_INLINE bool FindLayerInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName)
{
	return FindLayerIndexInList(layers, layerName) != -1;
}

static FORCE_INLINE bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName, const char*& foundLayer)
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

static FORCE_INLINE bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName)
{
	const char* dummy = nullptr;
	return FindLayerExtensionInList(layers, extensionName, dummy);
}

static FORCE_INLINE void TrimDuplicates(std::vector<const char*>& arr)
{
	for (int32 i = (int32)arr.size() - 1; i >= 0; --i)
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
	for (int32 i = 0; i < extensionProps.size(); ++i) {
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
	}
}

void VulkanLayerExtension::AddUniqueExtensionNames(std::vector<const char*>& outExtensions)
{
	for (int32 i = 0; i < extensionProps.size(); ++i) {
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
	}
}

void VulkanRHI::GetInstanceLayersAndExtensions(std::vector<const char*>& outInstanceExtensions, std::vector<const char*>& outInstanceLayers)
{
	std::vector<VulkanLayerExtension> globalLayerExtensions(1);
	EnumerateInstanceExtensionProperties(nullptr, globalLayerExtensions[0]);

	std::vector<std::string> foundUniqueExtensions;
	for (int32 i = 0; i < globalLayerExtensions[0].extensionProps.size(); ++i) {
		StringUtils::AddUnique(foundUniqueExtensions, globalLayerExtensions[0].extensionProps[i].extensionName);
	}
	
	uint32 instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	std::vector<VkLayerProperties> globalLayerProperties(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, globalLayerProperties.data());

	std::vector<std::string> foundUniqueLayers;
	for (int32 i = 0; i < globalLayerProperties.size(); ++i) 
	{
		VulkanLayerExtension layer;
		layer.layerProps = globalLayerProperties[i];
		EnumerateInstanceExtensionProperties(globalLayerProperties[i].layerName, layer);
		layer.AddUniqueExtensionNames(foundUniqueExtensions);
		StringUtils::AddUnique(foundUniqueLayers, globalLayerProperties[i].layerName);
		globalLayerExtensions.push_back(layer);
	}

	for (const std::string& name : foundUniqueLayers) {
		MLOG("- Found instance layer %s", name.c_str());
	}

	for (const std::string& name : foundUniqueExtensions) {
		MLOG("- Found instance extension %s", name.c_str());
	}
	
#if MONKEY_DEBUG
	for (int32 i = 0; G_ValidationLayersInstance[i] != nullptr; ++i) 
	{
		const char* currValidationLayer = G_ValidationLayersInstance[i];
		bool found = FindLayerInList(globalLayerExtensions, currValidationLayer);
		if (found) {
			outInstanceLayers.push_back(currValidationLayer);
		} 
		else {
			MLOG("Unable to find Vulkan instance validation layer '%s'", currValidationLayer);
		}
	}

	if (FindLayerExtensionInList(globalLayerExtensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
		outInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

#endif // MONKEY_DEBUG

	std::vector<const char*> platformExtensions;
	VulkanPlatform::GetInstanceExtensions(platformExtensions);

	for (const char* extension : platformExtensions) {
		if (FindLayerExtensionInList(globalLayerExtensions, extension)) {
			outInstanceExtensions.push_back(extension);
		}
	}

	for (int32 i = 0; G_InstanceExtensions[i] != nullptr; ++i) {
		if (FindLayerExtensionInList(globalLayerExtensions, G_InstanceExtensions[i])) {
			outInstanceExtensions.push_back(G_InstanceExtensions[i]);
		}
	}
    
	TrimDuplicates(outInstanceLayers);
	if (outInstanceLayers.size() > 0) {
		MLOG("Using instance layers");
		for (const char* layer : outInstanceLayers) {
			MLOG("* %s", layer);
		}
	}
	else {
		MLOG("Not using instance layers");
	}

	TrimDuplicates(outInstanceExtensions);
	if (outInstanceExtensions.size() > 0) {
		MLOG("Using instance extensions");
		for (const char* extension : outInstanceExtensions) {
			MLOG("* %s", extension);
		}
	}
	else {
		MLOG("Not using instance extensions");
	}
}

void VulkanDevice::GetDeviceExtensionsAndLayers(std::vector<const char*>& outDeviceExtensions, std::vector<const char*>& outDeviceLayers, bool& bOutDebugMarkers)
{
    bOutDebugMarkers = false;
    
    uint32 count = 0;
    vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &count, nullptr);
    std::vector<VkLayerProperties> properties(count);
    vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &count, properties.data());
    
    std::vector<VulkanLayerExtension> deviceLayerExtensions(count + 1);
    for (int32 index = 1; index < deviceLayerExtensions.size(); ++index) {
        deviceLayerExtensions[index].layerProps = properties[index - 1];
    }
    
    std::vector<std::string> foundUniqueLayers;
    std::vector<std::string> foundUniqueExtensions;
    for (int32 index = 0; index < deviceLayerExtensions.size(); ++index)
    {
        if (index == 0) {
            EnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, deviceLayerExtensions[index]);
        }
        else {
            StringUtils::AddUnique(foundUniqueLayers, deviceLayerExtensions[index].layerProps.layerName);
            EnumerateDeviceExtensionProperties(m_PhysicalDevice, deviceLayerExtensions[index].layerProps.layerName, deviceLayerExtensions[index]);
        }
        
        deviceLayerExtensions[index].AddUniqueExtensionNames(foundUniqueExtensions);
    }
    
    for (const std::string& name : foundUniqueLayers) {
        MLOG("- Found device layer %s", name.c_str());
    }
    
    for (const std::string& name : foundUniqueExtensions) {
        MLOG("- Found device extension %s", name.c_str());
    }

#if MONKEY_DEBUG
    for (uint32 layerIndex = 0; G_ValidationLayersDevice[layerIndex] != nullptr; ++layerIndex)
    {
        bool bValidationFound = false;
        const char* currValidationLayer = G_ValidationLayersDevice[layerIndex];
        for (int32 index = 1; index < deviceLayerExtensions.size(); ++index)
        {
            if (strcmp(deviceLayerExtensions[index].layerProps.layerName, currValidationLayer) == 0)
            {
                bValidationFound = true;
                outDeviceLayers.push_back(currValidationLayer);
                break;
            }
        }
        
        if (!bValidationFound) {
            MLOG("Unable to find Vulkan device validation layer '%s'", currValidationLayer);
        }
    }
#endif
    
    std::vector<const char*> availableExtensions;
    for (int32 extIndex = 0; extIndex < deviceLayerExtensions[0].extensionProps.size(); ++extIndex) {
        availableExtensions.push_back(deviceLayerExtensions[0].extensionProps[extIndex].extensionName);
    }
    
    for (int32 layerIndex = 0; layerIndex < outDeviceLayers.size(); ++layerIndex)
    {
        int32 findLayerIndex;
        for (findLayerIndex = 1; findLayerIndex < deviceLayerExtensions.size(); ++findLayerIndex)
        {
            if (strcmp(deviceLayerExtensions[findLayerIndex].layerProps.layerName, outDeviceLayers[layerIndex]) == 0) {
                break;
            }
        }
        
        if (findLayerIndex < deviceLayerExtensions.size()) {
            deviceLayerExtensions[findLayerIndex].AddUniqueExtensionNames(availableExtensions);
        }
    }
    
    TrimDuplicates(availableExtensions);
    
    auto ListContains = [](const std::vector<const char*>& arr, const char* name) -> bool
    {
        for (const char* element : arr)
        {
            if (strcmp(element, name) == 0) {
                return true;
            }
        }
        return false;
    };
    
    std::vector<const char*> platformExtensions;
    VulkanPlatform::GetDeviceExtensions(platformExtensions);
    for (const char* platformExtension : platformExtensions)
    {
        if (ListContains(availableExtensions, platformExtension))
        {
            outDeviceExtensions.push_back(platformExtension);
            break;
        }
    }
    
    for (uint32 index = 0; G_DeviceExtensions[index] != nullptr; ++index)
    {
        if (ListContains(availableExtensions, G_DeviceExtensions[index])) {
            outDeviceExtensions.push_back(G_DeviceExtensions[index]);
        }
    }
    
    if (outDeviceExtensions.size() > 0)
    {
        MLOG("Using device extensions");
        for (const char* extension : outDeviceExtensions) {
            MLOG("* %s", extension);
        }
    }
    
    if (outDeviceLayers.size() > 0)
    {
        MLOG("Using device layers");
        for (const char* layer : outDeviceLayers) {
            MLOG("* %s", layer);
        }
    }
}
