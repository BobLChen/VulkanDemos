#pragma once

#include "Engine.h"

#include "Common/Common.h"
#include "Vulkan/VulkanCommon.h"

#include <string>
#include <cstring>
#include <vector>
#include <memory>

class VulkanDevice;

namespace vk_demo
{
	
	class DVKBuffer
	{
	private:
		DVKBuffer()
		{

		}
	public:
		~DVKBuffer()
		{
			if (buffer != VK_NULL_HANDLE) {
				vkDestroyBuffer(device, buffer, nullptr);
				buffer = VK_NULL_HANDLE;
			}
			if (memory != VK_NULL_HANDLE) {
				vkFreeMemory(device, memory, nullptr);
				memory = VK_NULL_HANDLE;
			}
		}
	public:

		VkDevice				device = VK_NULL_HANDLE;

		VkBuffer				buffer = VK_NULL_HANDLE;
		VkDeviceMemory			memory = VK_NULL_HANDLE;

		VkDescriptorBufferInfo	descriptor;

		VkDeviceSize			size = 0;
		VkDeviceSize			alignment = 0;

		void*					mapped = nullptr;

		VkBufferUsageFlags		usageFlags;
		VkMemoryPropertyFlags	memoryPropertyFlags;

	public:

		static DVKBuffer* CreateBuffer(std::shared_ptr<VulkanDevice> device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);

		VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void UnMap();

		VkResult Bind(VkDeviceSize offset = 0);

		void SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		void CopyFrom(void* data, VkDeviceSize size);

		VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

		VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	};

};