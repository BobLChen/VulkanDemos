#include "DVKCommand.h"

#include "Vulkan/VulkanCommon.h"

namespace vk_demo
{
	DVKCommandBuffer::~DVKCommandBuffer()
	{
		VkDevice device = vulkanDevice->GetInstanceHandle();

		if (cmdBuffer != VK_NULL_HANDLE) 
		{
			vkFreeCommandBuffers(device, commandPool, 1, &cmdBuffer);
			cmdBuffer = VK_NULL_HANDLE;
		}

		if (fence != VK_NULL_HANDLE) 
		{
			vkDestroyFence(device, fence, nullptr);
			fence = VK_NULL_HANDLE;
		}

		queue = nullptr;
		vulkanDevice = nullptr;
	}

	DVKCommandBuffer::DVKCommandBuffer()
	{

	}

	void DVKCommandBuffer::Submit(VkSemaphore* signalSemaphore)
	{
		End();

		VkSubmitInfo submitInfo;
		ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
		submitInfo.commandBufferCount   = 1;
		submitInfo.pCommandBuffers      = &cmdBuffer;
		submitInfo.signalSemaphoreCount = signalSemaphore ? 1 : 0;
		submitInfo.pSignalSemaphores    = signalSemaphore;
		
		if (waitFlags.size() > 0) 
		{
			submitInfo.waitSemaphoreCount = waitSemaphores.size();
			submitInfo.pWaitSemaphores    = waitSemaphores.data();
			submitInfo.pWaitDstStageMask  = waitFlags.data();
		}

		vkResetFences(vulkanDevice->GetInstanceHandle(), 1, &fence);
		vkQueueSubmit(queue->GetHandle(), 1, &submitInfo, fence);
		vkWaitForFences(vulkanDevice->GetInstanceHandle(), 1, &fence, true, MAX_uint64);
	}

	void DVKCommandBuffer::Begin()
	{
		if (isBegun) {
			return;
		}
		isBegun = true;

		VkCommandBufferBeginInfo cmdBufBeginInfo;
		ZeroVulkanStruct(cmdBufBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdBuffer, &cmdBufBeginInfo);
	}

	void DVKCommandBuffer::End()
	{
		if (!isBegun) {
			return;
		}

		isBegun = false;
		vkEndCommandBuffer(cmdBuffer);
	}

	DVKCommandBuffer* DVKCommandBuffer::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkCommandPool commandPool, VkCommandBufferLevel level, std::shared_ptr<VulkanQueue> inQueue)
	{
		VkDevice device = vulkanDevice->GetInstanceHandle();

		DVKCommandBuffer* cmdBuffer = new DVKCommandBuffer();
		cmdBuffer->vulkanDevice = vulkanDevice;
		cmdBuffer->commandPool  = commandPool;
		cmdBuffer->isBegun      = false;

		if (inQueue) {
			cmdBuffer->queue = inQueue;
		}
		else {
			cmdBuffer->queue = vulkanDevice->GetGraphicsQueue();
		}

		VkCommandBufferAllocateInfo cmdBufferAllocateInfo;
		ZeroVulkanStruct(cmdBufferAllocateInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
		cmdBufferAllocateInfo.commandPool = commandPool;
		cmdBufferAllocateInfo.level       = level;
		cmdBufferAllocateInfo.commandBufferCount = 1;
		vkAllocateCommandBuffers(device, &cmdBufferAllocateInfo, &(cmdBuffer->cmdBuffer));

		VkFenceCreateInfo fenceCreateInfo;
		ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceCreateInfo.flags = 0;
		vkCreateFence(device, &fenceCreateInfo, nullptr, &(cmdBuffer->fence));

		return cmdBuffer;
	}

}
