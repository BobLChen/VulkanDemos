#pragma once

#include "Engine.h"
#include "DVKBuffer.h"
#include "DVKCommand.h"

#include "Common/Common.h"
#include "Math/Math.h"

#include "Vulkan/VulkanCommon.h"

#include <string>
#include <cstring>
#include <vector>
#include <memory>

namespace vk_demo
{
	
	class DVKIndexBuffer
	{
	private:
		DVKIndexBuffer()
		{

		}

	public:
		~DVKIndexBuffer()
		{
			if (dvkBuffer) {
				delete dvkBuffer;
			}
			dvkBuffer = nullptr;
		}

		void BindDraw(VkCommandBuffer cmdBuffer)
		{
			vkCmdBindIndexBuffer(cmdBuffer, dvkBuffer->buffer, 0, indexType);
			vkCmdDrawIndexed(cmdBuffer, indexCount, instanceCount, 0, 0, 0);
		}

		static DVKIndexBuffer* Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, std::vector<uint16> indices, VkIndexType type = VK_INDEX_TYPE_UINT16);

	public:
		VkDevice		device = VK_NULL_HANDLE;
		DVKBuffer*		dvkBuffer = nullptr;
        int32           instanceCount = 1;
		int32			indexCount = 0;
		VkIndexType		indexType = VK_INDEX_TYPE_UINT16;
	};

};
