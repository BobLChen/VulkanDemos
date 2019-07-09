#pragma once

#include "Common/Common.h"
#include "Math/Math.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanQueue.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanMemory.h"
#include "Engine.h"

#include <string>
#include <cstring>
#include <vector>
#include <stdarg.h>
#include <memory>

namespace vk_demo_util
{
	
	FORCEINLINE VkResult CreateBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr)
	{
		std::shared_ptr<VulkanDevice> device = Engine::Get()->GetVulkanRHI()->GetDevice();
		VkDevice handle = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();

		VkBufferCreateInfo bufferCreateInfo;
		ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VERIFYVULKANRESULT(vkCreateBuffer(handle, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, buffer));
		
		VkMemoryRequirements memReqs;
		VkMemoryAllocateInfo memAlloc;
		ZeroVulkanStruct(memAlloc, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

		vkGetBufferMemoryRequirements(handle, *buffer, &memReqs);
		memAlloc.allocationSize  = memReqs.size;
		device->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, memoryPropertyFlags, &memAlloc.memoryTypeIndex);
		VERIFYVULKANRESULT(vkAllocateMemory(handle, &memAlloc, VULKAN_CPU_ALLOCATOR, memory));
		
		if (data != nullptr)
		{
			void *mapped = nullptr;
			VERIFYVULKANRESULT(vkMapMemory(handle, *memory, 0, size, 0, &mapped));
			memcpy(mapped, data, size);

			if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange;
				ZeroVulkanStruct(mappedRange, VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size   = size;
				vkFlushMappedMemoryRanges(handle, 1, &mappedRange);
			}

			vkUnmapMemory(handle, *memory);
		}

		VERIFYVULKANRESULT(vkBindBufferMemory(handle, *buffer, *memory, 0));
		return VK_SUCCESS;
	}

	FORCEINLINE VkCommandBuffer CreateCommandBuffer(VkCommandPool commandPool, VkCommandBufferLevel level, bool begin = false)
	{
		std::shared_ptr<VulkanDevice> device = Engine::Get()->GetVulkanRHI()->GetDevice();
		VkDevice handle = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();

		VkCommandBufferAllocateInfo cmdBufAllocInfo;
		ZeroVulkanStruct(cmdBufAllocInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
		cmdBufAllocInfo.commandPool = commandPool;
		cmdBufAllocInfo.level = level;
		cmdBufAllocInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		VERIFYVULKANRESULT(vkAllocateCommandBuffers(handle, &cmdBufAllocInfo, &cmdBuffer));
		
		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufBeginInfo;
			ZeroVulkanStruct(cmdBufBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
			VERIFYVULKANRESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufBeginInfo));
		}

		return cmdBuffer;
	}

	FORCEINLINE void FlushCommandBuffer(VkCommandBuffer commandBuffer, VkCommandPool commandPool, VkQueue queue, bool free = true)
	{
		std::shared_ptr<VulkanDevice> device = Engine::Get()->GetVulkanRHI()->GetDevice();
		VkDevice handle = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();

		if (commandBuffer == VK_NULL_HANDLE)
		{
			return;
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));

		VkSubmitInfo submitInfo;
		ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &commandBuffer;

		VkFenceCreateInfo fenceInfo;
		ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceInfo.flags = 0;

		VkFence fence;
		VERIFYVULKANRESULT(vkCreateFence(handle, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));
			
		VERIFYVULKANRESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
		VERIFYVULKANRESULT(vkWaitForFences(handle, 1, &fence, VK_TRUE, MAX_int64));

		vkDestroyFence(handle, fence, nullptr);

		if (free)
		{
			vkFreeCommandBuffers(handle, commandPool, 1, &commandBuffer);
		}
	}

}