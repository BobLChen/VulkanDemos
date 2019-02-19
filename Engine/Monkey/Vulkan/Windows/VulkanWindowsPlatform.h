#define VK_USE_PLATFORM_WIN32_KHR					1
#define VULKAN_HAS_PHYSICAL_DEVICE_PROPERTIES2		1
#define VULKAN_USE_CREATE_WIN32_SURFACE				1
#define	VULKAN_SUPPORTS_DEDICATED_ALLOCATION		0
#define VULKAN_SUPPORTS_AMD_BUFFER_MARKER			1
#define VULKAN_SUPPORTS_NV_DIAGNOSTIC_CHECKPOINT	1

#include <vector>
#include <vulkan/vulkan.h>

#include "Common/Common.h"
#include "Vulkan/VulkanGenericPlatform.h"

class VulkanWindowsPlatform : public VulkanGenericPlatform
{
public:
	static bool LoadVulkanLibrary();

	static bool LoadVulkanInstanceFunctions(VkInstance instance);

	static void FreeVulkanLibrary();

	static void GetInstanceExtensions(std::vector<const char*>& outExtensions);

	static void GetDeviceExtensions(std::vector<const char*>& outExtensions);

	static void CreateSurface(VkInstance instance, VkSurfaceKHR* outSurface);

	static bool SupportsDeviceLocalHostVisibleWithNoPenalty();

	static void WriteCrashMarker(const OptionalVulkanDeviceExtensions& optionalExtensions, VkCommandBuffer cmdBuffer, VkBuffer destBuffer, const std::vector<uint32>& entries, bool adding);
};

typedef VulkanWindowsPlatform VulkanPlatform;
