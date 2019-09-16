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
	
	inline int32 VertexAttributeToSize(VertexAttribute attribute)
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
		else if (attribute == VertexAttribute::VA_SkinPack) {
			return 3 * sizeof(float);
		}
        else if (attribute == VertexAttribute::VA_Custom0 ||
                 attribute == VertexAttribute::VA_Custom1 ||
                 attribute == VertexAttribute::VA_Custom2 ||
                 attribute == VertexAttribute::VA_Custom3
        )
        {
            return 4 * sizeof(float);
        }
        else if (attribute == VertexAttribute::VA_InstanceFloat1) {
            return 1 * sizeof(float);
        }
        else if (attribute == VertexAttribute::VA_InstanceFloat2) {
            return 2 * sizeof(float);
        }
        else if (attribute == VertexAttribute::VA_InstanceFloat3) {
            return 3 * sizeof(float);
        }
        else if (attribute == VertexAttribute::VA_InstanceFloat4) {
            return 4 * sizeof(float);
        }
        
		return 0;
	}
    
	inline VkFormat VertexAttributeToVkFormat(VertexAttribute attribute)
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
		else if (attribute == VertexAttribute::VA_SkinPack) {
			format = VK_FORMAT_R32G32B32_SFLOAT;
		}
		else if (attribute == VertexAttribute::VA_SkinWeight) {
			format = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		else if (attribute == VertexAttribute::VA_SkinIndex) {
			format = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
        else if (attribute == VertexAttribute::VA_Custom0 ||
                 attribute == VertexAttribute::VA_Custom1 ||
                 attribute == VertexAttribute::VA_Custom2 ||
                 attribute == VertexAttribute::VA_Custom3
        )
        {
            format = VK_FORMAT_R32G32B32A32_SFLOAT;
        }
        else if (attribute == VertexAttribute::VA_InstanceFloat1) {
            format = VK_FORMAT_R32_SFLOAT;
        }
        else if (attribute == VertexAttribute::VA_InstanceFloat2) {
            format = VK_FORMAT_R32G32_SFLOAT;
        }
        else if (attribute == VertexAttribute::VA_InstanceFloat3) {
            format = VK_FORMAT_R32G32B32_SFLOAT;
        }
        else if (attribute == VertexAttribute::VA_InstanceFloat4) {
            format = VK_FORMAT_R32G32B32A32_SFLOAT;
        }
        
		return format;
	}
    
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

		std::vector<VkVertexInputAttributeDescription> GetInputAttributes(const std::vector<VertexAttribute>& shaderInputs);

		static DVKVertexBuffer* Create(std::shared_ptr<VulkanDevice> device, DVKCommandBuffer* cmdBuffer, std::vector<float> vertices, const std::vector<VertexAttribute>& attributes);

	public:
		VkDevice						device = VK_NULL_HANDLE;
		DVKBuffer*						dvkBuffer = nullptr;
		VkDeviceSize					offset = 0;
		std::vector<VertexAttribute>	attributes;
	};

};
