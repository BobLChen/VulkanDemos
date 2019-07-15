#include "DVKVertexBuffer.h"

namespace vk_demo
{
	int32 VertexAttributeToSize(VertexAttribute attribute)
	{
		// count * sizeof(float)
		if (attribute == VertexAttribute::VA_Position) {
			return 3 * sizeof(float);
		}
		else if (attribute == VertexAttribute::VA_UV0) {
			return 2 * sizeof(float);
		}
		else if (attribute == VertexAttribute::VA_UV1) {
			return 2 * sizeof(float);
		}
		else if (attribute == VertexAttribute::VA_Normal) {
			return 3 * sizeof(float);
		}
		else if (attribute == VertexAttribute::VA_Tangent) {
			return 4 * sizeof(float);
		}
		else if (attribute == VertexAttribute::VA_Color) {
			return 3 * sizeof(float);
		}
		else if (attribute == VertexAttribute::VA_SkinWeight) {
			return 4 * sizeof(float);
		}
		else if (attribute == VertexAttribute::VA_SkinIndex) {
			return 4 * sizeof(float);
		}
		return 0;
	}

	VkFormat VertexAttributeToVkFormat(VertexAttribute attribute)
	{
		VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
		if (attribute == VertexAttribute::VA_Position) {
			format = VK_FORMAT_R32G32B32_SFLOAT;
		}
		else if (attribute == VertexAttribute::VA_UV0) {
			format = VK_FORMAT_R32G32_SFLOAT;
		}
		else if (attribute == VertexAttribute::VA_UV1) {
			format = VK_FORMAT_R32G32_SFLOAT;
		}
		else if (attribute == VertexAttribute::VA_Normal) {
			format = VK_FORMAT_R32G32B32_SFLOAT;
		}
		else if (attribute == VertexAttribute::VA_Tangent) {
			format = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		else if (attribute == VertexAttribute::VA_Color) {
			format = VK_FORMAT_R32G32B32_SFLOAT;
		}
		else if (attribute == VertexAttribute::VA_SkinWeight) {
			format = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		else if (attribute == VertexAttribute::VA_SkinIndex) {
			format = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		return format;
	}

	DVKVertexBuffer* DVKVertexBuffer::Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, std::vector<float> vertices, std::vector<VertexAttribute> attributes)
	{
		VkDevice device = vulkanDevice->GetInstanceHandle();

		DVKVertexBuffer* vertexBuffer = new DVKVertexBuffer();
		vertexBuffer->device	 = device;
		vertexBuffer->attributes = attributes;

		vk_demo::DVKBuffer* vertexStaging = vk_demo::DVKBuffer::CreateBuffer(
			vulkanDevice, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			vertices.size() * sizeof(float), 
			vertices.data()
		);

		vertexBuffer->dvkBuffer = vk_demo::DVKBuffer::CreateBuffer(
			vulkanDevice, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			vertices.size() * sizeof(float)
		);

		cmdBuffer->Begin();

		VkBufferCopy copyRegion = {};
		copyRegion.size = vertices.size() * sizeof(float);
		vkCmdCopyBuffer(cmdBuffer->cmdBuffer, vertexStaging->buffer, vertexBuffer->dvkBuffer->buffer, 1, &copyRegion);

		cmdBuffer->End();
		cmdBuffer->Submit();

		delete vertexStaging;

		return vertexBuffer;
	}

	std::vector<VkVertexInputAttributeDescription> DVKVertexBuffer::GetInputAttributes(std::vector<VertexAttribute> shaderInputs)
	{
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
		int32 offset = 0;
		for (int32 i = 0; i < shaderInputs.size(); ++i)
		{
			VkVertexInputAttributeDescription inputAttribute = {};
			inputAttribute.binding  = 0;
			inputAttribute.location = i;
			inputAttribute.format   = VertexAttributeToVkFormat(shaderInputs[i]);
			inputAttribute.offset   = offset;
			offset += VertexAttributeToSize(shaderInputs[i]);
			vertexInputAttributs.push_back(inputAttribute);
		}
		return vertexInputAttributs;
	}

	VkVertexInputBindingDescription DVKVertexBuffer::GetInputBinding()
	{
		int32 stride = 0;
		for (int32 i = 0; i < attributes.size(); ++i) {
			stride += VertexAttributeToSize(attributes[i]);
		}

		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding   = 0;
		vertexInputBinding.stride    = stride;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vertexInputBinding;
	}

};