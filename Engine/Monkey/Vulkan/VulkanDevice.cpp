#include "VulkanDevice.h"
#include "VulkanPlatform.h"
#include "VulkanGlobals.h"
#include "VulkanFence.h"
#include "Application/Application.h"

VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice)
    : m_Device(VK_NULL_HANDLE)
    , m_PhysicalDevice(physicalDevice)
    , m_GfxQueue(nullptr)
    , m_ComputeQueue(nullptr)
    , m_TransferQueue(nullptr)
    , m_PresentQueue(nullptr)
    , m_FenceManager(nullptr)
    , m_MemoryManager(nullptr)
	, m_PhysicalDeviceFeatures2(nullptr)
{
    
}

VulkanDevice::~VulkanDevice()
{
    if (m_Device != VK_NULL_HANDLE)
    {
        Destroy();
        m_Device = VK_NULL_HANDLE;
    }
}

void VulkanDevice::CreateDevice()
{
	bool debugMarkersFound = false;
	std::vector<const char*> deviceExtensions;
	std::vector<const char*> validationLayers;
	GetDeviceExtensionsAndLayers(deviceExtensions, validationLayers, debugMarkersFound);

	if (m_AppDeviceExtensions.size() > 0)
	{
		MLOG("Using app device extensions");
		for (int32 i = 0; i < m_AppDeviceExtensions.size(); ++i)
		{
			deviceExtensions.push_back(m_AppDeviceExtensions[i]);
			MLOG("* %s", m_AppDeviceExtensions[i]);
		}
	}
	
    VkDeviceCreateInfo deviceInfo;
    ZeroVulkanStruct(deviceInfo, VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
	deviceInfo.enabledExtensionCount   = uint32_t(deviceExtensions.size());
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceInfo.enabledLayerCount       = uint32_t(validationLayers.size());
	deviceInfo.ppEnabledLayerNames     = validationLayers.data();

	if (m_PhysicalDeviceFeatures2) {
		deviceInfo.pNext            = m_PhysicalDeviceFeatures2;
		deviceInfo.pEnabledFeatures = nullptr;
		m_PhysicalDeviceFeatures2->features = m_PhysicalDeviceFeatures;
	}
	else {
		deviceInfo.pEnabledFeatures = &m_PhysicalDeviceFeatures;
	}

    MLOG("Found %d Queue Families", (int32)m_QueueFamilyProps.size());
    
	std::vector<VkDeviceQueueCreateInfo> queueFamilyInfos;
	
	uint32 numPriorities 		   = 0;
	int32 gfxQueueFamilyIndex 	   = -1;
	int32 computeQueueFamilyIndex  = -1;
	int32 transferQueueFamilyIndex = -1;
	
	for (int32 familyIndex = 0; familyIndex < m_QueueFamilyProps.size(); ++familyIndex)
	{
		const VkQueueFamilyProperties& currProps = m_QueueFamilyProps[familyIndex];
		bool isValidQueue = false;

		if ((currProps.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT)
		{
			if (gfxQueueFamilyIndex == -1) {
				gfxQueueFamilyIndex = familyIndex;
				isValidQueue = true;
			}
		}

		if ((currProps.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT)
		{
			if (computeQueueFamilyIndex == -1)
			{
				computeQueueFamilyIndex = familyIndex;
				isValidQueue = true;
			}
		}

		if ((currProps.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT)
		{
			if (transferQueueFamilyIndex == -1)
			{
				transferQueueFamilyIndex = familyIndex;
				isValidQueue = true;
			}
		}

		auto GetQueueInfoString = [](const VkQueueFamilyProperties& Props) -> std::string
		{
			std::string info;
			if ((Props.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
				info += " Gfx";
			}
			if ((Props.queueFlags & VK_QUEUE_COMPUTE_BIT) == VK_QUEUE_COMPUTE_BIT) {
				info += " Compute";
			}
			if ((Props.queueFlags & VK_QUEUE_TRANSFER_BIT) == VK_QUEUE_TRANSFER_BIT) {
				info += " Xfer";
			}
			if ((Props.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) == VK_QUEUE_SPARSE_BINDING_BIT) {
				info += " Sparse";
			}
			return info;
		};
		
		if (!isValidQueue)
		{
			MLOG("Skipping unnecessary Queue Family %d: %d queues%s", familyIndex, currProps.queueCount, GetQueueInfoString(currProps).c_str());
			continue;
		}

		VkDeviceQueueCreateInfo currQueue;
        ZeroVulkanStruct(currQueue, VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
		currQueue.queueFamilyIndex = familyIndex;
		currQueue.queueCount       = currProps.queueCount;
		numPriorities             += currProps.queueCount;
		queueFamilyInfos.push_back(currQueue);
        
		MLOG("Initializing Queue Family %d: %d queues%s", familyIndex,  currProps.queueCount, GetQueueInfoString(currProps).c_str());
	}
    
	std::vector<float> queuePriorities(numPriorities);
	float* CurrentPriority = queuePriorities.data();
	for (int32 index = 0; index < queueFamilyInfos.size(); ++index)
	{
		VkDeviceQueueCreateInfo& currQueue = queueFamilyInfos[index];
		currQueue.pQueuePriorities = CurrentPriority;
		const VkQueueFamilyProperties& currProps = m_QueueFamilyProps[currQueue.queueFamilyIndex];
		for (int32 queueIndex = 0; queueIndex < (int32)currProps.queueCount; ++queueIndex) {
			*CurrentPriority++ = 1.0f;
		}
	}

	deviceInfo.queueCreateInfoCount = uint32_t(queueFamilyInfos.size());
	deviceInfo.pQueueCreateInfos    = queueFamilyInfos.data();
	
	VkResult result = vkCreateDevice(m_PhysicalDevice, &deviceInfo, VULKAN_CPU_ALLOCATOR, &m_Device);
	if (result == VK_ERROR_INITIALIZATION_FAILED)
	{
		MLOG("%s", "Cannot create a Vulkan device. Try updating your video driver to a more recent version.\n");
		return;
	}

	m_GfxQueue = std::make_shared<VulkanQueue>(this, gfxQueueFamilyIndex);

	if (computeQueueFamilyIndex == -1) {
		computeQueueFamilyIndex = gfxQueueFamilyIndex;
	}
	m_ComputeQueue = std::make_shared<VulkanQueue>(this, computeQueueFamilyIndex);

	if (transferQueueFamilyIndex == -1) {
		transferQueueFamilyIndex = computeQueueFamilyIndex;
	}
	m_TransferQueue = std::make_shared<VulkanQueue>(this, transferQueueFamilyIndex);
}

void VulkanDevice::SetupFormats()
{
	for (uint32 index = 0; index < PF_MAX; ++index)
	{
		const VkFormat format = (VkFormat)index;
		memset(&m_FormatProperties[index], 0, sizeof(VkFormat));
		vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &m_FormatProperties[index]);
	}
    
	for (int32 index = 0; index < PF_MAX; ++index)
	{
		G_PixelFormats[index].platformFormat = VK_FORMAT_UNDEFINED;
		G_PixelFormats[index].supported = false;

		VkComponentMapping& componentMapping = m_PixelFormatComponentMapping[index];
		componentMapping.r = VK_COMPONENT_SWIZZLE_R;
		componentMapping.g = VK_COMPONENT_SWIZZLE_G;
		componentMapping.b = VK_COMPONENT_SWIZZLE_B;
		componentMapping.a = VK_COMPONENT_SWIZZLE_A;
	}

	MapFormatSupport(PF_B8G8R8A8, VK_FORMAT_B8G8R8A8_UNORM);
	SetComponentMapping(PF_B8G8R8A8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_G8, VK_FORMAT_R8_UNORM);
	SetComponentMapping(PF_G8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_G16, VK_FORMAT_R16_UNORM);
	SetComponentMapping(PF_G16, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_FloatRGB, VK_FORMAT_B10G11R11_UFLOAT_PACK32);
	SetComponentMapping(PF_FloatRGB, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_FloatRGBA, VK_FORMAT_R16G16B16A16_SFLOAT, 8);
	SetComponentMapping(PF_FloatRGBA, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);
    
    MapFormatSupport(PF_DepthStencil, VK_FORMAT_D32_SFLOAT_S8_UINT);
    if (!G_PixelFormats[PF_DepthStencil].supported)
    {
        MapFormatSupport(PF_DepthStencil, VK_FORMAT_D24_UNORM_S8_UINT);
        if (!G_PixelFormats[PF_DepthStencil].supported)
        {
            MapFormatSupport(PF_DepthStencil, VK_FORMAT_D16_UNORM_S8_UINT);
            if (!G_PixelFormats[PF_DepthStencil].supported) {
                MLOG("No stencil texture format supported!");
            }
        }
    }
	SetComponentMapping(PF_DepthStencil, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY);

	MapFormatSupport(PF_ShadowDepth, VK_FORMAT_D16_UNORM);
	SetComponentMapping(PF_ShadowDepth, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY);

	MapFormatSupport(PF_G32R32F, VK_FORMAT_R32G32_SFLOAT, 8);
	SetComponentMapping(PF_G32R32F, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_A32B32G32R32F, VK_FORMAT_R32G32B32A32_SFLOAT, 16);
	SetComponentMapping(PF_A32B32G32R32F, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_G16R16, VK_FORMAT_R16G16_UNORM);
	SetComponentMapping(PF_G16R16, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_G16R16F, VK_FORMAT_R16G16_SFLOAT);
	SetComponentMapping(PF_G16R16F, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_G16R16F_FILTER, VK_FORMAT_R16G16_SFLOAT);
	SetComponentMapping(PF_G16R16F_FILTER, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_R16_UINT, VK_FORMAT_R16_UINT);
	SetComponentMapping(PF_R16_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_R16_SINT, VK_FORMAT_R16_SINT);
	SetComponentMapping(PF_R16_SINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_R32_UINT, VK_FORMAT_R32_UINT);
	SetComponentMapping(PF_R32_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_R32_SINT, VK_FORMAT_R32_SINT);
	SetComponentMapping(PF_R32_SINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_R8_UINT, VK_FORMAT_R8_UINT);
	SetComponentMapping(PF_R8_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_D24, VK_FORMAT_D24_UNORM_S8_UINT);
    if (!G_PixelFormats[PF_D24].supported)
    {
        MapFormatSupport(PF_D24, VK_FORMAT_D16_UNORM_S8_UINT);
		if (!G_PixelFormats[PF_D24].supported)
		{
			MapFormatSupport(PF_D24, VK_FORMAT_D32_SFLOAT);
			if (!G_PixelFormats[PF_D24].supported)
			{
				MapFormatSupport(PF_D24, VK_FORMAT_D32_SFLOAT_S8_UINT);
				if (!G_PixelFormats[PF_D24].supported)
				{
					MapFormatSupport(PF_D24, VK_FORMAT_D16_UNORM);
                    if (!G_PixelFormats[PF_D24].supported) {
                        MLOG("%s", "No Depth texture format supported!");
                    }
				}
			}
		}
    }
	SetComponentMapping(PF_D24, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);
    
	MapFormatSupport(PF_R16F, VK_FORMAT_R16_SFLOAT);
	SetComponentMapping(PF_R16F, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_R16F_FILTER, VK_FORMAT_R16_SFLOAT);
	SetComponentMapping(PF_R16F_FILTER, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_FloatR11G11B10, VK_FORMAT_B10G11R11_UFLOAT_PACK32, 4);
	SetComponentMapping(PF_FloatR11G11B10, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_A2B10G10R10, VK_FORMAT_A2B10G10R10_UNORM_PACK32, 4);
	SetComponentMapping(PF_A2B10G10R10, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_A16B16G16R16, VK_FORMAT_R16G16B16A16_UNORM, 8);
	SetComponentMapping(PF_A16B16G16R16, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_A8, VK_FORMAT_R8_UNORM);
	SetComponentMapping(PF_A8, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_R);

	MapFormatSupport(PF_R5G6B5_UNORM, VK_FORMAT_R5G6B5_UNORM_PACK16);
	SetComponentMapping(PF_R5G6B5_UNORM, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_R8G8B8A8, VK_FORMAT_R8G8B8A8_UNORM);
	SetComponentMapping(PF_R8G8B8A8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_UINT);
	SetComponentMapping(PF_R8G8B8A8_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_SNORM);
	SetComponentMapping(PF_R8G8B8A8_SNORM, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_R16G16_UINT, VK_FORMAT_R16G16_UINT);
	SetComponentMapping(PF_R16G16_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_UINT);
	SetComponentMapping(PF_R16G16B16A16_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT);
	SetComponentMapping(PF_R16G16B16A16_SINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_UINT);
	SetComponentMapping(PF_R32G32B32A32_UINT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_SNORM);
	SetComponentMapping(PF_R16G16B16A16_SNORM, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_UNORM);
	SetComponentMapping(PF_R16G16B16A16_UNORM, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A);

	MapFormatSupport(PF_R8G8, VK_FORMAT_R8G8_UNORM);
	SetComponentMapping(PF_R8G8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_V8U8, VK_FORMAT_R8G8_UNORM);
	SetComponentMapping(PF_V8U8, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);

	MapFormatSupport(PF_R32_FLOAT, VK_FORMAT_R32_SFLOAT);
	SetComponentMapping(PF_R32_FLOAT, VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO, VK_COMPONENT_SWIZZLE_ZERO);
}

void VulkanDevice::MapFormatSupport(PixelFormat format, VkFormat vkFormat)
{
	PixelFormatInfo& formatInfo = G_PixelFormats[format];
	formatInfo.platformFormat   = vkFormat;
	formatInfo.supported        = IsFormatSupported(vkFormat);

	if (!formatInfo.supported) {
		MLOG("PixelFormat(%d) is not supported with Vk format %d", (int32)format, (int32)vkFormat);
	}
}

void VulkanDevice::SetComponentMapping(PixelFormat format, VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a)
{
	VkComponentMapping& componentMapping = m_PixelFormatComponentMapping[format];
	componentMapping.r = r;
	componentMapping.g = g;
	componentMapping.b = b;
	componentMapping.a = a;
}

void VulkanDevice::MapFormatSupport(PixelFormat format, VkFormat vkFormat, int32 blockBytes)
{
	MapFormatSupport(format, vkFormat);
	PixelFormatInfo& formatInfo = G_PixelFormats[format];
	formatInfo.blockBytes = blockBytes;
}

bool VulkanDevice::QueryGPU(int32 deviceIndex)
{
	bool discrete = false;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);

	auto GetDeviceTypeString = [&]() -> std::string
	{
		std::string info;
		switch (m_PhysicalDeviceProperties.deviceType)
		{
		case  VK_PHYSICAL_DEVICE_TYPE_OTHER:
			info = ("Other");
			break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			info = ("Integrated GPU");
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			info = ("Discrete GPU");
			discrete = true;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			info = ("Virtual GPU");
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			info = ("CPU");
			break;
		default:
			info = ("Unknown");
			break;
		}
		return info;
	};

	MLOG("Device %d: %s", deviceIndex, m_PhysicalDeviceProperties.deviceName);
	MLOG("- API %d.%d.%d(0x%x) Driver 0x%x VendorId 0x%x", VK_VERSION_MAJOR(m_PhysicalDeviceProperties.apiVersion), VK_VERSION_MINOR(m_PhysicalDeviceProperties.apiVersion), VK_VERSION_PATCH(m_PhysicalDeviceProperties.apiVersion), m_PhysicalDeviceProperties.apiVersion, m_PhysicalDeviceProperties.driverVersion, m_PhysicalDeviceProperties.vendorID);
	MLOG("- DeviceID 0x%x Type %s", m_PhysicalDeviceProperties.deviceID, GetDeviceTypeString().c_str());
	MLOG("- Max Descriptor Sets Bound %d Timestamps %d", m_PhysicalDeviceProperties.limits.maxBoundDescriptorSets, m_PhysicalDeviceProperties.limits.timestampComputeAndGraphics);

	uint32 queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, nullptr);
	m_QueueFamilyProps.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueCount, m_QueueFamilyProps.data());
    
    return discrete;
}

void VulkanDevice::InitGPU(int32 deviceIndex)
{
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &m_PhysicalDeviceFeatures);
	MLOG("Using Device %d: Geometry %d Tessellation %d", deviceIndex, m_PhysicalDeviceFeatures.geometryShader, m_PhysicalDeviceFeatures.tessellationShader);

	CreateDevice();
	SetupFormats();
    
    m_MemoryManager = new VulkanDeviceMemoryManager();
    m_MemoryManager->Init(this);
    
    m_FenceManager = new VulkanFenceManager();
	m_FenceManager->Init(this);
}

void VulkanDevice::Destroy()
{
	m_FenceManager->Destory();
	delete m_FenceManager;

	m_MemoryManager->Destory();
	delete m_MemoryManager;

	vkDestroyDevice(m_Device, VULKAN_CPU_ALLOCATOR);
	m_Device = VK_NULL_HANDLE;
}

bool VulkanDevice::IsFormatSupported(VkFormat format)
{
	auto ArePropertiesSupported = [](const VkFormatProperties& prop) -> bool 
	{
		return (prop.bufferFeatures != 0) || (prop.linearTilingFeatures != 0) || (prop.optimalTilingFeatures != 0);
	};

	if (format >= 0 && format < PF_MAX)
	{
		const VkFormatProperties& prop = m_FormatProperties[format];
		return ArePropertiesSupported(prop);
	}
    
	auto it = m_ExtensionFormatProperties.find(format);
	if (it != m_ExtensionFormatProperties.end())
	{
		const VkFormatProperties& foundProperties = it->second;
		return ArePropertiesSupported(foundProperties);
	}
    
	VkFormatProperties newProperties;
	memset(&newProperties, 0, sizeof(VkFormatProperties));
	
	vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &newProperties);
    m_ExtensionFormatProperties.insert(std::pair<VkFormat, VkFormatProperties>(format, newProperties));
    
	return ArePropertiesSupported(newProperties);
}

const VkComponentMapping& VulkanDevice::GetFormatComponentMapping(PixelFormat format) const
{
    return m_PixelFormatComponentMapping[format];
}
