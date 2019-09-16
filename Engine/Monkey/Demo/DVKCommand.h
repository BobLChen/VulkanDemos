#pragma once

#include "Engine.h"

#include "Common/Common.h"
#include "Math/Math.h"

#include "Vulkan/VulkanCommon.h"

#include <string>
#include <cstring>
#include <vector>
#include <memory>

class VulkanDevice;

namespace vk_demo
{

    class DVKCommandBuffer
	{
	public:
		~DVKCommandBuffer();

	private:
		DVKCommandBuffer();

	public:
		void Begin();

		void End();

		void Submit(VkSemaphore* signalSemaphore = nullptr);

		static DVKCommandBuffer* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, std::shared_ptr<VulkanQueue> queue = nullptr);

	public:
		std::shared_ptr<VulkanQueue>		queue = nullptr;

		VkCommandBuffer						cmdBuffer = VK_NULL_HANDLE;
		VkFence								fence = VK_NULL_HANDLE;
		VkCommandPool						commandPool = VK_NULL_HANDLE;
		std::shared_ptr<VulkanDevice>		vulkanDevice = nullptr;
		std::vector<VkPipelineStageFlags>	waitFlags;
		std::vector<VkSemaphore>			waitSemaphores;

		bool								isBegun;
	};

}
