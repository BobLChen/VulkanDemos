#include "VulkanDevice.h"
#include "VulkanMemory.h"
#include "VulkanSwapChain.h"
#include "Math/Math.h"

VulkanSwapChain::VulkanSwapChain(VkInstance instance, std::shared_ptr<VulkanDevice> device, PixelFormat& outPixelFormat, uint32 width, uint32 height,
	uint32* outDesiredNumBackBuffers, std::vector<VkImage>& outImages, int8 lockToVsync)
	: m_SwapChain(VK_NULL_HANDLE)
    , m_Surface(VK_NULL_HANDLE)
	, m_Device(device)
	, m_CurrentImageIndex(-1)
	, m_SemaphoreIndex(0)
	, m_NumPresentCalls(0)
	, m_NumAcquireCalls(0)
	, m_Instance(instance)
	, m_LockToVsync(lockToVsync)
{
	VulkanPlatform::CreateSurface(instance, &m_Surface);
	
	VkSurfaceFormatKHR currFormat;
	memset(&currFormat, 0, sizeof(VkSurfaceFormatKHR));

	uint32 numFormats;
	VERIFYVULKANRESULT_EXPANDED(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFormats, nullptr));
	std::vector<VkSurfaceFormatKHR> formats(numFormats);
	VERIFYVULKANRESULT_EXPANDED(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFormats, formats.data()));
	
	if (outPixelFormat != PF_Unknown)
	{
		bool bFound = false;
		if (G_PixelFormats[outPixelFormat].supported)
		{
			VkFormat requested = (VkFormat)G_PixelFormats[outPixelFormat].platformFormat;
			for (int32 index = 0; index < formats.size(); ++index)
			{
				if (formats[index].format == requested)
				{
					currFormat = formats[index];
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				MLOG("Requested PixelFormat %d not supported by this swapchain! Falling back to supported swapchain format...", (uint32)outPixelFormat);
				outPixelFormat = PF_Unknown;
			}
		}
		else
		{
			MLOG("Requested PixelFormat %d not supported by this Vulkan implementation!", (uint32)outPixelFormat);
			outPixelFormat = PF_Unknown;
		}
	}

	if (outPixelFormat == PF_Unknown)
	{
		for (int32 index = 0; index < formats.size(); ++index)
		{
			for (int32 pfIndex = 0; pfIndex < PF_MAX; ++pfIndex)
			{
				if (formats[index].format == G_PixelFormats[pfIndex].platformFormat)
				{
					outPixelFormat = (PixelFormat)pfIndex;
					currFormat = formats[index];
					MLOG("No swapchain format requested, picking up VulkanFormat %d", (uint32)currFormat.format);
					break;
				}
			}
			
			if (outPixelFormat != PF_Unknown)
			{
				break;
			}
		}
	}
	
	if (outPixelFormat == PF_Unknown)
	{
		MLOG("Can't find a proper pixel format for the swapchain, trying to pick up the first available");
		VkFormat platformFormat = PixelFormatToVkFormat(outPixelFormat, false);
		bool supported = false;
		for (int32 index = 0; index < formats.size(); ++index)
		{
			if (formats[index].format == platformFormat)
			{
				supported = true;
				currFormat = formats[index];
			}
		}
	}
	
	m_Device->SetupPresentQueue(m_Surface);
    
    uint32 numFoundPresentModes = 0;
    VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFoundPresentModes, nullptr));
    std::vector< VkPresentModeKHR> foundPresentModes(numFoundPresentModes);
    VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFoundPresentModes, foundPresentModes.data()));
    
    bool foundPresentModeMailbox = false;
    bool foundPresentModeImmediate = false;
    bool foundPresentModeFIFO = false;
    
    MLOG("Found %d present mode.", numFoundPresentModes);
    for (int32 index = 0; index < numFoundPresentModes; ++index)
    {
        switch (foundPresentModes[index])
        {
            case VK_PRESENT_MODE_MAILBOX_KHR:
                foundPresentModeMailbox = true;
                MLOG("- VK_PRESENT_MODE_MAILBOX_KHR (%d)", (int32)VK_PRESENT_MODE_MAILBOX_KHR);
                break;
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                foundPresentModeImmediate = true;
                MLOG("- VK_PRESENT_MODE_IMMEDIATE_KHR (%d)", (int32)VK_PRESENT_MODE_IMMEDIATE_KHR);
                break;
            case VK_PRESENT_MODE_FIFO_KHR:
                foundPresentModeFIFO = true;
                MLOG("- VK_PRESENT_MODE_FIFO_KHR (%d)", (int32)VK_PRESENT_MODE_FIFO_KHR);
                break;
            default:
                MLOG("- VkPresentModeKHR (%d)", (int32)foundPresentModes[index]);
                break;
        }
    }
    
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (foundPresentModeImmediate && !m_LockToVsync)
    {
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    else if (foundPresentModeMailbox)
    {
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else if (foundPresentModeFIFO)
    {
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else
    {
        MLOG("Couldn't find desired PresentMode! Using %d", (int32)foundPresentModes[0]);
        presentMode = foundPresentModes[0];
    }
    
    MLOG("Selected VkPresentModeKHR mode %d", presentMode);

	VkSurfaceCapabilitiesKHR surfProperties;
	VERIFYVULKANRESULT_EXPANDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->GetPhysicalHandle(), m_Surface, &surfProperties));
	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfProperties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfProperties.currentTransform;
	}

	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	if (surfProperties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
	{
		compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	
	uint32 desiredNumBuffers = surfProperties.maxImageCount > 0 ? MMath::Clamp(*outDesiredNumBackBuffers, surfProperties.minImageCount, surfProperties.maxImageCount) : *outDesiredNumBackBuffers;
    uint32 sizeX = surfProperties.currentExtent.width  == 0xFFFFFFFF ? width : surfProperties.currentExtent.width;
	uint32 sizeY = surfProperties.currentExtent.height == 0xFFFFFFFF ? height : surfProperties.currentExtent.height;
	
	VkSwapchainCreateInfoKHR swapChainInfo;
	ZeroVulkanStruct(swapChainInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
	swapChainInfo.surface = m_Surface;
	swapChainInfo.minImageCount = desiredNumBuffers;
	swapChainInfo.imageFormat = currFormat.format;
	swapChainInfo.imageColorSpace = currFormat.colorSpace;
	swapChainInfo.imageExtent.width = sizeX;
	swapChainInfo.imageExtent.height = sizeY;
	swapChainInfo.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	swapChainInfo.preTransform = preTransform;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainInfo.presentMode = presentMode;
	swapChainInfo.oldSwapchain = VK_NULL_HANDLE;
	swapChainInfo.clipped = VK_TRUE;
	swapChainInfo.compositeAlpha = compositeAlpha;
	
	*outDesiredNumBackBuffers = desiredNumBuffers;

	if (swapChainInfo.imageExtent.width == 0)
	{
		swapChainInfo.imageExtent.width = width;
	}
	if (swapChainInfo.imageExtent.height == 0)
	{
		swapChainInfo.imageExtent.height = height;
	}

	VkBool32 supportsPresent;
	VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfaceSupportKHR(m_Device->GetPhysicalHandle(), m_Device->GetPresentQueue()->GetFamilyIndex(), m_Surface, &supportsPresent));

	VERIFYVULKANRESULT(vkCreateSwapchainKHR(m_Device->GetInstanceHandle(), &swapChainInfo, VULKAN_CPU_ALLOCATOR, &m_SwapChain));

	uint32 numSwapChainImages;
	VERIFYVULKANRESULT(vkGetSwapchainImagesKHR(m_Device->GetInstanceHandle(), m_SwapChain, &numSwapChainImages, nullptr));
	outImages.resize(numSwapChainImages);
	VERIFYVULKANRESULT(vkGetSwapchainImagesKHR(m_Device->GetInstanceHandle(), m_SwapChain, &numSwapChainImages, outImages.data()));

	m_ImageAcquiredFences.resize(numSwapChainImages);
	VulkanFenceManager& fenceMgr = m_Device->GetFenceManager();
	for (int32 index = 0; index < numSwapChainImages; ++index)
	{
		m_ImageAcquiredFences[index] = fenceMgr.CreateFence(true);
	}

	m_ImageAcquiredSemaphore.resize(numSwapChainImages);
	for (int32 index = 0; index < numSwapChainImages; ++index)
	{
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		VERIFYVULKANRESULT(vkCreateSemaphore(m_Device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_ImageAcquiredSemaphore[index]));
	}
	
	m_PresentID = 0;
}

void VulkanSwapChain::Destroy()
{
	vkDestroySwapchainKHR(m_Device->GetInstanceHandle(), m_SwapChain, VULKAN_CPU_ALLOCATOR);
	m_SwapChain = VK_NULL_HANDLE;

	VulkanFenceManager& fenceMgr = m_Device->GetFenceManager();
	for (int32 index = 0; index < m_ImageAcquiredFences.size(); ++index)
	{
		fenceMgr.ReleaseFence(m_ImageAcquiredFences[index]);
	}

	for (int32 index = 0; index < m_ImageAcquiredSemaphore.size(); ++index)
	{
		vkDestroySemaphore(m_Device->GetInstanceHandle(), m_ImageAcquiredSemaphore[index], VULKAN_CPU_ALLOCATOR);
		m_ImageAcquiredSemaphore[index] = VK_NULL_HANDLE;
	}

	vkDestroySurfaceKHR(m_Instance, m_Surface, VULKAN_CPU_ALLOCATOR);
	m_Surface = VK_NULL_HANDLE;
}

int32 VulkanSwapChain::AcquireImageIndex(VkSemaphore* outSemaphore)
{
	uint32 imageIndex = 0;
	const int32 prevSemaphoreIndex = m_SemaphoreIndex;
	m_SemaphoreIndex = (m_SemaphoreIndex + 1) % m_ImageAcquiredSemaphore.size();
	
	VulkanFenceManager& fenceMgr = m_Device->GetFenceManager();
	fenceMgr.ResetFence(m_ImageAcquiredFences[m_SemaphoreIndex]);
	const VkFence acquiredFence = m_ImageAcquiredFences[m_SemaphoreIndex]->GetHandle();

	VkResult result = vkAcquireNextImageKHR(
		m_Device->GetInstanceHandle(),
		m_SwapChain, 
		UINT64_MAX, 
		m_ImageAcquiredSemaphore[m_SemaphoreIndex], 
		acquiredFence,
		&imageIndex
	);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		m_SemaphoreIndex = prevSemaphoreIndex;
		return (int32)Status::OutOfDate;
	}

	if (result == VK_ERROR_SURFACE_LOST_KHR)
	{
		m_SemaphoreIndex = prevSemaphoreIndex;
		return (int32)Status::SurfaceLost;
	}

	++m_NumAcquireCalls;
	*outSemaphore = m_ImageAcquiredSemaphore[m_SemaphoreIndex];
	m_CurrentImageIndex = (int32)imageIndex;

	fenceMgr.WaitForFence(m_ImageAcquiredFences[m_SemaphoreIndex], UINT64_MAX);
	
	return m_CurrentImageIndex;
}

VulkanSwapChain::Status VulkanSwapChain::Present(VulkanQueue* gfxQueue, VulkanQueue* presentQueue, VkSemaphore* backBufferRenderingDoneSemaphore)
{
	if (m_CurrentImageIndex == -1)
	{
		return Status::Healthy;
	}

	VkPresentInfoKHR createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
	VkSemaphore Semaphore = VK_NULL_HANDLE;
	if (backBufferRenderingDoneSemaphore)
	{
		createInfo.waitSemaphoreCount = 1;
		Semaphore = *backBufferRenderingDoneSemaphore;
		createInfo.pWaitSemaphores = &Semaphore;
	}

	createInfo.swapchainCount = 1;
	createInfo.pSwapchains = &m_SwapChain;
	createInfo.pImageIndices = (uint32*)&m_CurrentImageIndex;
    
	m_PresentID++;

	VkResult presentResult = vkQueuePresentKHR(presentQueue->GetHandle(), &createInfo);

	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR)
	{
		return Status::OutOfDate;
	}

	if (presentResult == VK_ERROR_SURFACE_LOST_KHR)
	{
		return Status::SurfaceLost;
	}

	if (presentResult != VK_SUCCESS && presentResult != VK_SUBOPTIMAL_KHR)
	{
		VERIFYVULKANRESULT(presentResult);
	}

	++m_NumPresentCalls;

	return Status::Healthy;
}


void VulkanDevice::SetupPresentQueue(VkSurfaceKHR surface)
{
	if (!m_PresentQueue)
	{
		const auto SupportsPresent = [surface](VkPhysicalDevice physicalDevice, std::shared_ptr<VulkanQueue> queue)
		{
			VkBool32 supportsPresent = VK_FALSE;
			const uint32 familyIndex = queue->GetFamilyIndex();
			VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, familyIndex, surface, &supportsPresent));
			if (supportsPresent)
			{
				MLOG("Queue Family %d: Supports Present", familyIndex);
			}
			return (supportsPresent == VK_TRUE);
		};

		// bool gfx = SupportsPresent(m_PhysicalDevice, m_GfxQueue);
		bool compute = SupportsPresent(m_PhysicalDevice, m_ComputeQueue);
		if (m_TransferQueue->GetFamilyIndex() != m_GfxQueue->GetFamilyIndex() && m_TransferQueue->GetFamilyIndex() != m_ComputeQueue->GetFamilyIndex())
		{
			SupportsPresent(m_PhysicalDevice, m_TransferQueue);
		}

		if (m_ComputeQueue->GetFamilyIndex() != m_GfxQueue->GetFamilyIndex() && compute)
		{
			m_PresentQueue = m_ComputeQueue;
		}
		else
		{
			m_PresentQueue = m_GfxQueue;
		}
	}
}
