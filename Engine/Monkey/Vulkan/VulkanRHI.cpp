#include "VulkanRHI.h"
#include "VulkanPlatform.h"
#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/SlateApplication.h"
#include "VulkanCommon.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"
#include "VulkanMemory.h"
#include "VulkanSwapChain.h"

static inline int32 PreferAdapterVendor()
{
    return -1;
}

VulkanRHI::VulkanRHI()
	: m_Instance(VK_NULL_HANDLE)
	, m_PresentComplete(VK_NULL_HANDLE)
	, m_RenderComplete(VK_NULL_HANDLE)
	, m_CommandPool(VK_NULL_HANDLE)
	, m_SubmitPipelineStages(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
	, m_Device(nullptr)
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
#if MONKEY_DEBUG
    RemoveDebugLayerCallback();
#endif
	DestoryEvent();

	DestorySwapChain();

    m_Device->Destroy();
    m_Device = nullptr;
    
    vkDestroyInstance(m_Instance, VULKAN_CPU_ALLOCATOR);
}

void VulkanRHI::InitInstance()
{
	CreateInstance();
#if MONKEY_DEBUG
    SetupDebugLayerCallback();
#endif
    SelectAndInitDevice();

	InitEvent();

	RecreateSwapChain();

	uint32 DesiredNumBackBuffers = 3;
	std::vector<VkImage> images;
	PixelFormat pixelFormat = PF_R8G8B8A8;
	VulkanSwapChain* swapChain = new VulkanSwapChain(m_Instance, m_Device, pixelFormat, 800, 600, &DesiredNumBackBuffers, images, 1);
}

void VulkanRHI::InitEvent()
{
	VkSemaphoreCreateInfo createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
	vkCreateSemaphore(m_Device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_PresentComplete);
	vkCreateSemaphore(m_Device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);

	ZeroVulkanStruct(m_SubmitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
	m_SubmitInfo.pWaitDstStageMask = &m_SubmitPipelineStages;
	m_SubmitInfo.waitSemaphoreCount = 1;
	m_SubmitInfo.pWaitSemaphores = &m_PresentComplete;
	m_SubmitInfo.signalSemaphoreCount = 1;
	m_SubmitInfo.pSignalSemaphores = &m_RenderComplete;
}

void VulkanRHI::DestoryEvent()
{
	vkDestroySemaphore(m_Device->GetInstanceHandle(), m_PresentComplete, VULKAN_CPU_ALLOCATOR);
	vkDestroySemaphore(m_Device->GetInstanceHandle(), m_RenderComplete, VULKAN_CPU_ALLOCATOR);
}

void VulkanRHI::RecreateSwapChain()
{

}

void VulkanRHI::DestorySwapChain()
{

}

void VulkanRHI::SavePipelineCache()
{

}

void VulkanRHI::PooledUniformBuffersBeginFrame()
{

}

void VulkanRHI::ReleasePooledUniformBuffers()
{

}

void VulkanRHI::CreateInstance()
{
	GetInstanceLayersAndExtensions(m_InstanceExtensions, m_InstanceLayers, m_SupportsDebugUtilsExt);

	VkApplicationInfo appInfo;
	ZeroVulkanStruct(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
	appInfo.pApplicationName = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetTitle();
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo instanceCreateInfo;
	ZeroVulkanStruct(instanceCreateInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = m_InstanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = m_InstanceExtensions.size() > 0 ? m_InstanceExtensions.data() : nullptr;
	instanceCreateInfo.enabledLayerCount = m_InstanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = m_InstanceLayers.size() > 0 ? m_InstanceLayers.data() : nullptr;

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
	else if (result != VK_SUCCESS)
	{
		MLOG("Vulkan failed to create instance.");
	}
	else
	{
		MLOG("Vulkan successed to create instance.");
	}
    
    if (result != VK_SUCCESS)
    {
        SlateApplication::Get().OnRequestingExit();
    }

}

void VulkanRHI::SelectAndInitDevice()
{
    uint32 gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);

    if (result == VK_ERROR_INITIALIZATION_FAILED)
    {
        MLOG("%s\n", "Cannot find a compatible Vulkan device or driver. Try updating your video driver to a more recent version and make sure your video card supports Vulkan.");
        SlateApplication::Get().OnRequestingExit();
        return;
    }
    
    if (gpuCount == 0)
    {
        MLOG("%s\n", "Couldn't enumerate physical devices! Make sure your drivers are up to date and that you are not pending a reboot.");
        SlateApplication::Get().OnRequestingExit();
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
		m_Devices.push_back(newDevice);

		bool isDiscrete = newDevice->QueryGPU(index);
        if (isDiscrete) {
            discreteDevices.push_back({newDevice, index});
        }
        else {
            integratedDevices.push_back({newDevice, index});
        }
	}
    
    for (int32 index = 0; index < integratedDevices.size(); ++index)
    {
        discreteDevices.push_back(integratedDevices[index]);
    }
    
    int32 deviceIndex = -1;
    if (discreteDevices.size() > 0)
    {
        int32 preferredVendor = PreferAdapterVendor();
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
        SlateApplication::Get().OnRequestingExit();
        return;
    }
	
    m_Device->InitGPU(deviceIndex);
}