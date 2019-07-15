#pragma once

#include "Engine.h"
#include "DVKCommand.h"
#include "DVKBuffer.h"

#include "Common/Common.h"
#include "Math/Math.h"

#include "Vulkan/VulkanCommon.h"

#include <string>
#include <cstring>
#include <vector>
#include <memory>

namespace vk_demo
{
	
	class DVKVertexBuffer
	{
	private:
		DVKVertexBuffer()
		{

		}

	public:
		~DVKVertexBuffer()
		{
			if (dvkBuffer) {
				delete dvkBuffer;
			}
			dvkBuffer = nullptr;
		}

		void Bind(VkCommandBuffer cmdBuffer)
		{
			vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &(dvkBuffer->buffer), &offset);
		}

		VkVertexInputBindingDescription GetInputBinding();

		std::vector<VkVertexInputAttributeDescription> GetInputAttributes(std::vector<VertexAttribute> shaderInputs);

		static DVKVertexBuffer* Create(std::shared_ptr<VulkanDevice> device, DVKCommandBuffer* cmdBuffer, std::vector<float> vertices, std::vector<VertexAttribute> attributes);

	public:
		VkDevice						device = VK_NULL_HANDLE;
		DVKBuffer*						dvkBuffer = nullptr;
		VkDeviceSize					offset = 0;
		std::vector<VertexAttribute>	attributes;
	};

};