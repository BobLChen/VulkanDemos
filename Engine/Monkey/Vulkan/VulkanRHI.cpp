
#include "Common/Common.h"
#include "Common/Log.h"

#include "Engine.h"
#include "Application/Application.h"
#include "Application/GenericApplication.h"

#include "RHIDefinitions.h"
#include "VulkanRHI.h"
#include "VulkanPlatform.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"
#include "VulkanMemory.h"
#include "VulkanSwapChain.h"
#include "VulkanMemory.h"

VulkanRHI::VulkanRHI()
	: m_Instance(VK_NULL_HANDLE)
	, m_Device(nullptr)
	, m_SwapChain(nullptr)
    , m_PixelFormat(PF_B8G8R8A8)
{
	
}

VulkanRHI::~VulkanRHI()
{

}

void VulkanRHI::Init()
{
	if (!VulkanPlatform::LoadVulkanLibrary()) {
		MLOG("%s\n", "Failed load vulkan libarary.");
		return;
	}

	InitInstance();
}

void VulkanRHI::PostInit()
{

}

void VulkanRHI::Shutdown()
{
    DestorySwapChain();
#if MONKEY_DEBUG
    RemoveDebugLayerCallback();
#endif
	m_Device->Destroy();
    m_Device = nullptr;
    vkDestroyInstance(m_Instance, VULKAN_CPU_ALLOCATOR);
}

void VulkanRHI::InitInstance()
{
	CreateInstance();
	if (!VulkanPlatform::LoadVulkanInstanceFunctions(m_Instance)) {
		MLOG("%s\n", "Failed load vulkan instance functions.");
		return;
	}
#if MONKEY_DEBUG
    SetupDebugLayerCallback();
#endif
    SelectAndInitDevice();
    RecreateSwapChain();
}

void VulkanRHI::RecreateSwapChain()
{
	DestorySwapChain();

    uint32 desiredNumBackBuffers = 3;
    int32 width  = Engine::Get()->GetPlatformWindow()->GetWidth();
    int32 height = Engine::Get()->GetPlatformWindow()->GetHeight();
    m_SwapChain  = std::shared_ptr<VulkanSwapChain>(new VulkanSwapChain(m_Instance, m_Device, m_PixelFormat, width, height, &desiredNumBackBuffers, m_BackbufferImages, 1));
	
	m_BackbufferViews.resize(m_BackbufferImages.size());
	for (int32 i = 0; i < m_BackbufferViews.size(); ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo;
        ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
        imageViewCreateInfo.format     = PixelFormatToVkFormat(m_PixelFormat, false);
        imageViewCreateInfo.components = m_Device->GetFormatComponentMapping(m_PixelFormat);
        imageViewCreateInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.image      = m_BackbufferImages[i];
        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;
        VERIFYVULKANRESULT(vkCreateImageView(m_Device->GetInstanceHandle(), &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &(m_BackbufferViews[i])));
    }

}

void VulkanRHI::DestorySwapChain()
{
    m_SwapChain = nullptr;

	for (int32 i = 0; i < m_BackbufferViews.size(); ++i) {
		vkDestroyImageView(m_Device->GetInstanceHandle(), m_BackbufferViews[i], VULKAN_CPU_ALLOCATOR);
	}
}

void VulkanRHI::CreateInstance()
{
	GetInstanceLayersAndExtensions(m_InstanceExtensions, m_InstanceLayers);
	
	if (m_AppInstanceExtensions.size() > 0)
	{
		MLOG("Using app instance extensions");
		for (int32 i = 0; i < m_AppInstanceExtensions.size(); ++i)
		{
			m_InstanceExtensions.push_back(m_AppInstanceExtensions[i]);
			MLOG("* %s", m_AppInstanceExtensions[i]);
		}
	}
	
	VkApplicationInfo appInfo;
	ZeroVulkanStruct(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
	appInfo.pApplicationName   = Engine::Get()->GetTitle();
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName        = ENGINE_NAME;
	appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 0);
    
#if PLATFORM_IOS || PLATFORM_ANDROID
    appInfo.apiVersion         = VK_API_VERSION_1_0;
#else
    appInfo.apiVersion         = VK_API_VERSION_1_1;
#endif
	
	VkInstanceCreateInfo instanceCreateInfo;
	ZeroVulkanStruct(instanceCreateInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	instanceCreateInfo.pApplicationInfo        = &appInfo;
	instanceCreateInfo.enabledExtensionCount   = uint32_t(m_InstanceExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = m_InstanceExtensions.size() > 0 ? m_InstanceExtensions.data() : nullptr;
	instanceCreateInfo.enabledLayerCount       = uint32_t(m_InstanceLayers.size());
	instanceCreateInfo.ppEnabledLayerNames     = m_InstanceLayers.size() > 0 ? m_InstanceLayers.data() : nullptr;

	VkResult result = vkCreateInstance(&instanceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Instance);
	if (result == VK_ERROR_INCOMPATIBLE_DRIVER) 
	{
		MLOG("%s", "Cannot find a compatible Vulkan driver (ICD).");
	}
	else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		std::string missingExtensions;
		uint32 propertyCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data());

		for (const char* extension : m_InstanceExtensions)
		{
			bool found = false;
			for (uint32 i = 0; i < propertyCount; ++i)
			{
				const char* propExtension = properties[i].extensionName;
				if (strcmp(propExtension, extension) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				std::string extensionStr(extension);
				missingExtensions += extensionStr + "\n";
			}
		}

		MLOG("Vulkan driver doesn't contain specified extensions:\n%s", missingExtensions.c_str());
	}
	else if (result != VK_SUCCESS) {
		MLOG("Vulkan failed to create instance.");
	}
	else {
		MLOG("Vulkan successed to create instance.");
	}
}

void VulkanRHI::SelectAndInitDevice()
{
	uint32_t gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);

    if (result == VK_ERROR_INITIALIZATION_FAILED) {
        MLOG("%s\n", "Cannot find a compatible Vulkan device or driver. Try updating your video driver to a more recent version and make sure your video card supports Vulkan.");
        return;
    }
    
    if (gpuCount == 0) {
        MLOG("%s\n", "Couldn't enumerate physical devices! Make sure your drivers are up to date and that you are not pending a reboot.");
        return;
    }

	MLOG("Found %d device(s)", gpuCount);

    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    vkEnumeratePhysicalDevices(m_Instance, &gpuCount, physicalDevices.data());
	
	struct DeviceInfo
	{
		std::shared_ptr<VulkanDevice> device;
		uint32 deviceIndex;
	};
	std::vector<DeviceInfo> discreteDevices;
	std::vector<DeviceInfo> integratedDevices;

	for (uint32 index = 0; index < gpuCount; ++index)
	{
		std::shared_ptr<VulkanDevice> newDevice = std::make_shared<VulkanDevice>(physicalDevices[index]);
		bool isDiscrete = newDevice->QueryGPU(index);
        if (isDiscrete) {
            discreteDevices.push_back({newDevice, index});
        }
        else {
            integratedDevices.push_back({newDevice, index});
        }
	}
    
    for (int32 index = 0; index < integratedDevices.size(); ++index) {
        discreteDevices.push_back(integratedDevices[index]);
    }
    
    int32 deviceIndex = -1;
    if (discreteDevices.size() > 0)
    {
		int32 preferredVendor = -1;
        if (discreteDevices.size() > 1 && preferredVendor != -1)
        {
            for (int32 index = 0; index < discreteDevices.size(); ++index)
            {
                if (discreteDevices[index].device->GetDeviceProperties().vendorID == preferredVendor)
                {
                    m_Device = discreteDevices[index].device;
                    deviceIndex = discreteDevices[index].deviceIndex;
                    break;
                }
            }
        }
        
        if (deviceIndex == -1)
        {
            m_Device = discreteDevices[0].device;
            deviceIndex = discreteDevices[0].deviceIndex;
        }
    }
    else
    {
        MLOG("%s", "No devices found!");
        deviceIndex = -1;
        return;
    }

	for (int32 i = 0; i < m_AppDeviceExtensions.size(); ++i)
	{
		m_Device->AddAppDeviceExtensions(m_AppDeviceExtensions[i]);
	}

	m_Device->SetPhysicalDeviceFeatures(m_PhysicalDeviceFeatures2);

    m_Device->InitGPU(deviceIndex);
}
