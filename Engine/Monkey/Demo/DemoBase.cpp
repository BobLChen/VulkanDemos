#include "DemoBase.h"

void DemoBase::Setup()
{
	auto vulkanRHI    = GetVulkanRHI();
	auto vulkanDevice = vulkanRHI->GetDevice();

	m_SwapChain     = vulkanRHI->GetSwapChain();

	m_Device		= vulkanDevice->GetInstanceHandle();
	m_VulkanDevice	= vulkanDevice;
	m_GfxQueue		= vulkanDevice->GetGraphicsQueue()->GetHandle();
	m_PresentQueue	= vulkanDevice->GetPresentQueue()->GetHandle();

	m_FrameWidth	= vulkanRHI->GetSwapChain()->GetWidth();
	m_FrameHeight	= vulkanRHI->GetSwapChain()->GetHeight();

	m_PixelFormat	= vulkanRHI->GetPixelFormat();
	m_DepthFormat   = PF_D24;

	m_SampleCount   = VK_SAMPLE_COUNT_1_BIT;
}

void DemoBase::Present()
{
	int32 backBufferIndex = m_SwapChain->AcquireImageIndex(&m_PresentComplete);
    
	VkSubmitInfo submitInfo = {};
	submitInfo.sType 				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pWaitDstStageMask 	= &m_WaitStageMask;									
	submitInfo.pWaitSemaphores 		= &m_PresentComplete;
	submitInfo.waitSemaphoreCount 	= 1;
	submitInfo.pSignalSemaphores 	= &m_RenderComplete;
	submitInfo.signalSemaphoreCount = 1;											
	submitInfo.pCommandBuffers 		= &(m_CommandBuffers[backBufferIndex]);
	submitInfo.commandBufferCount 	= 1;												
	
	// 提交绘制命令
    vkResetFences(m_Device, 1, &(m_Fences[backBufferIndex]));

	VERIFYVULKANRESULT(vkQueueSubmit(m_GfxQueue, 1, &submitInfo, m_Fences[backBufferIndex]));
    vkWaitForFences(m_Device, 1, &(m_Fences[backBufferIndex]), true, 200 * 1000 * 1000);
    
    // present
    m_SwapChain->Present(m_VulkanDevice->GetGraphicsQueue(), m_VulkanDevice->GetPresentQueue(), &m_RenderComplete);
}

uint32 DemoBase::GetMemoryTypeFromProperties(uint32 typeBits, VkMemoryPropertyFlags properties)
{
	uint32 memoryTypeIndex = 0;
	GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(typeBits, properties, &memoryTypeIndex);
	return memoryTypeIndex;
}

void DemoBase::DestroyPipelineCache()
{
	VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
	vkDestroyPipelineCache(device, m_PipelineCache, VULKAN_CPU_ALLOCATOR);
	m_PipelineCache = VK_NULL_HANDLE;
}

void DemoBase::CreatePipelineCache()
{
	VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();

	VkPipelineCacheCreateInfo createInfo;
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
    VERIFYVULKANRESULT(vkCreatePipelineCache(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineCache));
}

void DemoBase::CreateFences()
{
	VkDevice device  = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    int32 frameCount = GetVulkanRHI()->GetSwapChain()->GetBackBufferCount();
        
	VkFenceCreateInfo fenceCreateInfo;
	ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        
    m_Fences.resize(frameCount);
	for (int32 i = 0; i < m_Fences.size(); ++i) {
		VERIFYVULKANRESULT(vkCreateFence(device, &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
	}

	VkSemaphoreCreateInfo createInfo;
	ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
	vkCreateSemaphore(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
}

void DemoBase::DestroyFences()
{
	VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();

	for (int32 i = 0; i < m_Fences.size(); ++i) {
		vkDestroyFence(device, m_Fences[i], VULKAN_CPU_ALLOCATOR);
	}

	vkDestroySemaphore(device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
}

void DemoBase::CreateCommandBuffers()
{
	VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
	VkCommandPoolCreateInfo cmdPoolInfo;
	ZeroVulkanStruct(cmdPoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
	cmdPoolInfo.queueFamilyIndex = GetVulkanRHI()->GetDevice()->GetPresentQueue()->GetFamilyIndex();
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VERIFYVULKANRESULT(vkCreateCommandPool(device, &cmdPoolInfo, VULKAN_CPU_ALLOCATOR, &m_CommandPool));
        
    VkCommandBufferAllocateInfo cmdBufferInfo;
    ZeroVulkanStruct(cmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    cmdBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmdBufferInfo.commandBufferCount = 1;
    cmdBufferInfo.commandPool        = m_CommandPool;
        
    m_CommandBuffers.resize(GetVulkanRHI()->GetSwapChain()->GetBackBufferCount());
    for (int32 i = 0; i < m_CommandBuffers.size(); ++i) {
        vkAllocateCommandBuffers(device, &cmdBufferInfo, &(m_CommandBuffers[i]));
    }
}

void DemoBase::DestroyCommandBuffers()
{
	VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    for (int32 i = 0; i < m_CommandBuffers.size(); ++i) {
        vkFreeCommandBuffers(device, m_CommandPool, 1, &(m_CommandBuffers[i]));
    }

	vkDestroyCommandPool(device, m_CommandPool, VULKAN_CPU_ALLOCATOR);
}
