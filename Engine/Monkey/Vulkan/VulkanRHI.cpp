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
#include "VulkanMemory.h"

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
    , m_SwapChain(nullptr)
    , m_DepthStencilImage(VK_NULL_HANDLE)
    , m_DepthStencilView(VK_NULL_HANDLE)
    , m_DepthStencilMemory(VK_NULL_HANDLE)
    , m_PixelFormat(PF_B8G8R8A8)
    , m_DepthFormat(VK_FORMAT_D24_UNORM_S8_UINT)
    , m_RenderPass(VK_NULL_HANDLE)
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
	DestroyFrameBuffer();
    DestoryPipelineCache();
    DestoryRenderPass();
    DestoryDepthStencil();
    DestoryCommandBuffers();
    DestoryCommandPool();
    DestorySwapChain();
	DestorySemaphore();
    
    m_Device->Destroy();
    m_Device = nullptr;
    
#if MONKEY_DEBUG
    RemoveDebugLayerCallback();
#endif
    vkDestroyInstance(m_Instance, VULKAN_CPU_ALLOCATOR);
}

void VulkanRHI::InitInstance()
{
	CreateInstance();
#if MONKEY_DEBUG
    SetupDebugLayerCallback();
#endif
    SelectAndInitDevice();
	CreateSemaphores();
    RecreateSwapChain();
    CreateCommandPool();
    CreateCommandBuffers();
    CreateDepthStencil();
    CreateRenderPass();
    CreatePipelineCache();
	CreateFrameBuffer();
}

void VulkanRHI::CreateFrameBuffer()
{
	int width = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetWidth();
	int height = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetHeight();

	VkImageView attachments[2];
	attachments[1] = m_DepthStencilView;

	VkFramebufferCreateInfo frameBufferCreateInfo;
	ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
	frameBufferCreateInfo.renderPass = m_RenderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width  = width;
	frameBufferCreateInfo.height = height;
	frameBufferCreateInfo.layers = 1;

	m_FrameBuffers.resize(m_FrameImageViews.size());
	for (uint32 i = 0; i < m_FrameBuffers.size(); ++i)
	{
		attachments[0] = m_FrameImageViews[i];
		VERIFYVULKANRESULT(vkCreateFramebuffer(m_Device->GetInstanceHandle(), &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
	}
}

void VulkanRHI::DestroyFrameBuffer()
{
	for (uint32 i = 0; i < m_FrameBuffers.size(); ++i) {
		vkDestroyFramebuffer(m_Device->GetInstanceHandle(), m_FrameBuffers[i], VULKAN_CPU_ALLOCATOR);
	}
}

void VulkanRHI::CreatePipelineCache()
{
    VkPipelineCacheCreateInfo createInfo;
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
    VERIFYVULKANRESULT(vkCreatePipelineCache(m_Device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineCache));
}

void VulkanRHI::DestoryPipelineCache()
{
    vkDestroyPipelineCache(m_Device->GetInstanceHandle(), m_PipelineCache, VULKAN_CPU_ALLOCATOR);
}

void VulkanRHI::CreateRenderPass()
{
    std::vector<VkAttachmentDescription> attachments(2);
    // color attachment
    attachments[0].format = PixelFormatToVkFormat(m_PixelFormat, false);
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // depth stencil attachment
    attachments[1].format = m_DepthFormat;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference colorReference = { };
    colorReference.attachment = 0;
    colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthReference = { };
    depthReference.attachment = 1;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpassDescription = { };
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.pResolveAttachments = nullptr;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = nullptr;
    
    std::vector<VkSubpassDependency> dependencies(2);
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    VkRenderPassCreateInfo renderPassInfo;
    ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies = dependencies.data();
    
    VERIFYVULKANRESULT(vkCreateRenderPass(m_Device->GetInstanceHandle(), &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));
}

void VulkanRHI::DestoryRenderPass()
{
    vkDestroyRenderPass(m_Device->GetInstanceHandle(), m_RenderPass, VULKAN_CPU_ALLOCATOR);
}

void VulkanRHI::CreateSemaphores()
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

void VulkanRHI::DestorySemaphore()
{
	vkDestroySemaphore(m_Device->GetInstanceHandle(), m_PresentComplete, VULKAN_CPU_ALLOCATOR);
	vkDestroySemaphore(m_Device->GetInstanceHandle(), m_RenderComplete, VULKAN_CPU_ALLOCATOR);
}

void VulkanRHI::CreateCommandPool()
{
    VkCommandPoolCreateInfo createInfo;
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
    createInfo.queueFamilyIndex = m_Device->GetPresentQueue()->GetFamilyIndex();
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VERIFYVULKANRESULT(vkCreateCommandPool(m_Device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_CommandPool));
}

void VulkanRHI::DestoryCommandPool()
{
    vkDestroyCommandPool(m_Device->GetInstanceHandle(), m_CommandPool, VULKAN_CPU_ALLOCATOR);
}

void VulkanRHI::RecreateSwapChain()
{
    uint32 desiredNumBackBuffers = 3;
    int width  = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetWidth();
    int height = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetHeight();
    
    m_SwapChain = std::shared_ptr<VulkanSwapChain>(new VulkanSwapChain(m_Instance, m_Device, m_PixelFormat, width, height, &desiredNumBackBuffers, m_FrameImages, 1));
    
    m_FrameImageViews.resize(m_FrameImages.size());
    for (int i = 0; i < m_FrameImages.size(); ++i)
    {
        VkImageViewCreateInfo createInfo;
        ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
        createInfo.format = PixelFormatToVkFormat(m_PixelFormat, false);
        createInfo.components = m_Device->GetFormatComponentMapping(m_PixelFormat);
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.flags = 0;
        createInfo.image = m_FrameImages[i];
        VERIFYVULKANRESULT(vkCreateImageView(m_Device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &(m_FrameImageViews[i])));
    }
}

void VulkanRHI::DestorySwapChain()
{
    for (int i = 0; i < m_FrameImageViews.size(); ++i)
    {
        vkDestroyImageView(m_Device->GetInstanceHandle(), m_FrameImageViews[i], VULKAN_CPU_ALLOCATOR);
    }
    m_SwapChain->Destroy();
    m_SwapChain = nullptr;
}

void VulkanRHI::CreateCommandBuffers()
{
    m_CommandBuffers.resize(m_FrameImageViews.size());
    VkCommandBufferAllocateInfo allocateInfo;
    ZeroVulkanStruct(allocateInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    allocateInfo.commandPool = m_CommandPool;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = m_FrameImageViews.size();
    
    VERIFYVULKANRESULT(vkAllocateCommandBuffers(m_Device->GetInstanceHandle(), &allocateInfo, m_CommandBuffers.data()));
}

void VulkanRHI::DestoryCommandBuffers()
{
    vkFreeCommandBuffers(m_Device->GetInstanceHandle(), m_CommandPool, m_FrameImageViews.size(), m_CommandBuffers.data());
}

void VulkanRHI::CreateDepthStencil()
{
    int width  = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetWidth();
    int height = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetHeight();
    
    VkImageCreateInfo imageCreateInfo;
    ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = m_DepthFormat;
    imageCreateInfo.extent = { (uint32)width, (uint32)height, 1 };
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageCreateInfo.flags = 0;
    VERIFYVULKANRESULT(vkCreateImage(m_Device->GetInstanceHandle(), &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilImage));
    
    VkImageViewCreateInfo imageViewCreateInfo;
    ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;
    imageViewCreateInfo.image = m_DepthStencilImage;
    
    VkMemoryRequirements memRequire;
    vkGetImageMemoryRequirements(m_Device->GetInstanceHandle(), imageViewCreateInfo.image, &memRequire);
    
    uint32 memoryTypeIndex = 0;
    VERIFYVULKANRESULT(m_Device->GetMemoryManager().GetMemoryTypeFromProperties(memRequire.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex));
    
    VkMemoryAllocateInfo memAllocateInfo;
    ZeroVulkanStruct(memAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    memAllocateInfo.allocationSize = memRequire.size;
    memAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    
    vkAllocateMemory(m_Device->GetInstanceHandle(), &memAllocateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilMemory);
    vkBindImageMemory(m_Device->GetInstanceHandle(), m_DepthStencilImage, m_DepthStencilMemory, 0);
    VERIFYVULKANRESULT(vkCreateImageView(m_Device->GetInstanceHandle(), &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilView));
}

void VulkanRHI::DestoryDepthStencil()
{
    vkFreeMemory(m_Device->GetInstanceHandle(), m_DepthStencilMemory, VULKAN_CPU_ALLOCATOR);
    vkDestroyImageView(m_Device->GetInstanceHandle(), m_DepthStencilView, VULKAN_CPU_ALLOCATOR);
    vkDestroyImage(m_Device->GetInstanceHandle(), m_DepthStencilImage, VULKAN_CPU_ALLOCATOR);
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
