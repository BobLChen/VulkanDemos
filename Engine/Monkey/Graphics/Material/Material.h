#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanPipeline.h"
#include "Vulkan/VulkanResources.h"
#include "Graphics/Shader/Shader.h"
#include <memory>

class Material
{
public:
	struct UniformBufferParam
	{
		std::string				name;
		uint32					set;
		uint32					binding;
		VulkanUniformBuffer*	buffer;
		int32					descriptorIndex;
	};

public:
    Material(std::shared_ptr<Shader> shader);

    virtual ~Material();
    
	inline void SetShader(std::shared_ptr<Shader> shader)
	{
		m_Shader = shader;
		GenerateShaderParams();
	}

	inline std::shared_ptr<Shader> GetShader() const
	{
		return m_Shader;
	}

    // ----------------------------------- VkPipelineInputAssemblyStateCreateInfo -----------------------------------

	inline void SetTopology(VkPrimitiveTopology topology)
	{
		if (m_StateInfo.inputAssemblyState.topology == topology)
		{
			return;
		}
		m_StateInfo.inputAssemblyState.topology = topology;
		MarkStateDirty();
	}

	inline VkPrimitiveTopology GetTopology() const
	{
		return m_StateInfo.inputAssemblyState.topology;
	}

    // ----------------------------------- VkPipelineRasterizationStateCreateInfo -----------------------------------

	inline void SetpolygonMode(VkPolygonMode polygonMode)
	{
		if (m_StateInfo.rasterizationState.polygonMode == polygonMode) 
		{
			return;
		}
		m_StateInfo.rasterizationState.polygonMode = polygonMode;
		MarkStateDirty();
	}

	inline VkPolygonMode GetPolygonMode() const
	{
		return m_StateInfo.rasterizationState.polygonMode;
	}
    
	inline void SetCullMode(VkCullModeFlags cullMode)
	{
		if (m_StateInfo.rasterizationState.cullMode == cullMode)
		{
			return;
		}
		m_StateInfo.rasterizationState.cullMode = cullMode;
		MarkStateDirty();
	}

	inline VkCullModeFlags GetCullMode() const
	{
		return m_StateInfo.rasterizationState.cullMode;
	}

	inline void SetFrontFace(VkFrontFace frontFace)
	{
		if (m_StateInfo.rasterizationState.frontFace == frontFace)
		{
			return;
		}
		m_StateInfo.rasterizationState.frontFace = frontFace;
		MarkStateDirty();
	}

	inline VkFrontFace GetFrontFace() const
	{
		return m_StateInfo.rasterizationState.frontFace;
	}

	inline void SetDepthClampEnable(VkBool32 enable)
	{
		if (m_StateInfo.rasterizationState.depthClampEnable == enable)
		{
			return;
		}
		m_StateInfo.rasterizationState.depthClampEnable = enable;
		MarkStateDirty();
	}

	inline VkBool32 GetDepthClampEnable() const
	{
		return m_StateInfo.rasterizationState.depthClampEnable;
	}

	inline void SetDiscardEnable(VkBool32 enable)
	{
		if (m_StateInfo.rasterizationState.rasterizerDiscardEnable == enable)
		{
			return;
		}
		m_StateInfo.rasterizationState.rasterizerDiscardEnable = enable;
		MarkStateDirty();
	}

	inline VkBool32 GetDiscardEnable() const
	{
		return m_StateInfo.rasterizationState.rasterizerDiscardEnable;
	}

	inline void SetDepthBiasEnable(VkBool32 enable)
	{
		if (m_StateInfo.rasterizationState.depthBiasEnable == enable)
		{
			return;
		}
		m_StateInfo.rasterizationState.depthBiasEnable = enable;
		MarkStateDirty();
	}

	inline VkBool32 GetDepthBiasEnable() const
	{
		return m_StateInfo.rasterizationState.depthBiasEnable;
	}

	inline void SetLineWidth(float width)
	{
		if (m_StateInfo.rasterizationState.lineWidth == width)
		{
			return;
		}
		m_StateInfo.rasterizationState.lineWidth = width;
		MarkStateDirty();
	}

	inline float GetLineWidth() const
	{
		return m_StateInfo.rasterizationState.lineWidth;
	}

    // ----------------------------------- VkPipelineColorBlendAttachmentState -----------------------------------

	inline void SetColorWriteMask(VkColorComponentFlags mask)
	{
		if (m_StateInfo.colorBlendAttachmentState.colorWriteMask == mask)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.colorWriteMask = mask;
		MarkStateDirty();
	}

	inline VkColorComponentFlags GetColorWriteMask() const
	{
		return m_StateInfo.colorBlendAttachmentState.colorWriteMask;
	}

	inline void SetBlendEnable(VkBool32 enable)
	{
		if (m_StateInfo.colorBlendAttachmentState.blendEnable == enable)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.blendEnable = enable;
		MarkStateDirty();
	}

	inline VkBool32 GetBlendEnable() const
	{
		return m_StateInfo.colorBlendAttachmentState.blendEnable;
	}

	inline void SetSrcColorBlendFactor(VkBlendFactor op)
	{
		if (m_StateInfo.colorBlendAttachmentState.srcColorBlendFactor == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.srcColorBlendFactor = op;
		MarkStateDirty();
	}

	inline VkBlendFactor GetSrcColorBlendFactor() const
	{
		return m_StateInfo.colorBlendAttachmentState.srcColorBlendFactor;
	}

	inline void SetDstColorBlendFactor(VkBlendFactor op)
	{
		if (m_StateInfo.colorBlendAttachmentState.dstColorBlendFactor == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.dstColorBlendFactor = op;
		MarkStateDirty();
	}

	inline VkBlendFactor GetDstColorBlendFactor() const
	{
		return m_StateInfo.colorBlendAttachmentState.dstColorBlendFactor;
	}

	inline void SetColorBlendOp(VkBlendOp op)
	{
		if (m_StateInfo.colorBlendAttachmentState.colorBlendOp == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.colorBlendOp = op;
		MarkStateDirty();
	}

	inline VkBlendOp GetColorBlendOp() const
	{
		return m_StateInfo.colorBlendAttachmentState.colorBlendOp;
	}

	inline void SetSrcAlphaBlendFactor(VkBlendFactor op)
	{
		if (m_StateInfo.colorBlendAttachmentState.srcAlphaBlendFactor == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.srcAlphaBlendFactor = op;
		MarkStateDirty();
	}

	inline VkBlendFactor GetSrcAlphaBlendFactor() const
	{
		return m_StateInfo.colorBlendAttachmentState.srcAlphaBlendFactor;
	}

	inline void SetDstAlphaBlendFactor(VkBlendFactor op)
	{
		if (m_StateInfo.colorBlendAttachmentState.dstAlphaBlendFactor == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.dstAlphaBlendFactor = op;
		MarkStateDirty();
	}

	inline VkBlendFactor GetDstAlphaBlendFactor() const
	{
		return m_StateInfo.colorBlendAttachmentState.dstAlphaBlendFactor;
	}

	inline void SetAlphaBlendOp(VkBlendOp op)
	{
		if (m_StateInfo.colorBlendAttachmentState.alphaBlendOp == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.alphaBlendOp = op;
		MarkStateDirty();
	}

	inline VkBlendOp GetAlphaBlendOp() const
	{
		return m_StateInfo.colorBlendAttachmentState.alphaBlendOp;
	}

    // ----------------------------------- VkPipelineDepthStencilStateCreateInfo -----------------------------------

	inline void SetDepthTestEnable(VkBool32 enable)
	{
		if (m_StateInfo.depthStencilState.depthTestEnable == enable)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthTestEnable = enable;
		MarkStateDirty();
	}

	inline VkBool32 GetDepthTestEnable() const
	{
		return m_StateInfo.depthStencilState.depthTestEnable;
	}

	inline void SetDepthWriteEnable(VkBool32 enable)
	{
		if (m_StateInfo.depthStencilState.depthWriteEnable == enable)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthWriteEnable = enable;
		MarkStateDirty();
	}

	inline VkBool32 GetDepthWriteEnable() const
	{
		return m_StateInfo.depthStencilState.depthWriteEnable;
	}

	inline void SetDepthCompareOp(VkCompareOp op)
	{
		if (m_StateInfo.depthStencilState.depthCompareOp == op)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthCompareOp = op;
		MarkStateDirty();
	}

	inline VkCompareOp GetDepthCompareOp() const
	{
		return m_StateInfo.depthStencilState.depthCompareOp;
	}

	inline void SetDepthBoundsTestEnable(VkBool32 enable)
	{
		if (m_StateInfo.depthStencilState.depthBoundsTestEnable == enable)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthBoundsTestEnable = enable;
		MarkStateDirty();
	}

	inline VkBool32 GetDepthBoundsTestEnable() const
	{
		return m_StateInfo.depthStencilState.depthBoundsTestEnable;
	}

	inline void SetStencilTestEnable(VkBool32 enable)
	{
		if (m_StateInfo.depthStencilState.depthTestEnable == enable)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthTestEnable = enable;
		MarkStateDirty();
	}

	inline VkBool32 GetStencilTestEnable() const
	{
		return m_StateInfo.depthStencilState.depthTestEnable;
	}

	inline void SetDepthStencilCompareMask(bool front, uint32 mask)
	{
		if (front)
		{
			if (m_StateInfo.depthStencilState.front.compareMask == mask)
			{
				return;
			}
			m_StateInfo.depthStencilState.front.compareMask = mask;
		}
		else
		{
			if (m_StateInfo.depthStencilState.back.compareMask == mask)
			{
				return;
			}
			m_StateInfo.depthStencilState.back.compareMask = mask;
		}
		MarkStateDirty();
	}

	inline uint32 GetDepthStencilCompareMask(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.compareMask : m_StateInfo.depthStencilState.back.compareMask;
	}

	inline void SetDepthStencilCompareOp(bool front, VkCompareOp op)
	{
		if (front)
		{
			if (m_StateInfo.depthStencilState.front.compareOp == op)
			{
				return;
			}
			m_StateInfo.depthStencilState.front.compareOp = op;
		}
		else
		{
			if (m_StateInfo.depthStencilState.back.compareOp == op)
			{
				return;
			}
			m_StateInfo.depthStencilState.back.compareOp = op;
		}
		MarkStateDirty();
	}

	inline VkCompareOp GetDepthStencilCompareOp(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.compareOp : m_StateInfo.depthStencilState.back.compareOp;
	}

	inline void SetStencilFailOp(bool front, VkStencilOp op)
	{
		if (front)
		{
			if (m_StateInfo.depthStencilState.front.failOp == op)
			{
				return;
			}
			m_StateInfo.depthStencilState.front.failOp = op;
		}
		else
		{
			if (m_StateInfo.depthStencilState.back.failOp == op)
			{
				return;
			}
			m_StateInfo.depthStencilState.back.failOp = op;
		}
		MarkStateDirty();
	}

	inline VkStencilOp GetStencilFailOp(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.failOp : m_StateInfo.depthStencilState.back.failOp;
	}

	inline void SetStencilPassOp(bool front, VkStencilOp op)
	{
		if (front)
		{
			if (m_StateInfo.depthStencilState.front.passOp == op)
			{
				return;
			}
			m_StateInfo.depthStencilState.front.passOp = op;
		}
		else
		{
			if (m_StateInfo.depthStencilState.back.passOp == op)
			{
				return;
			}
			m_StateInfo.depthStencilState.front.passOp = op;
		}
		MarkStateDirty();
	}

	inline VkStencilOp GetStencilPassOp(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.passOp : m_StateInfo.depthStencilState.back.passOp;
	}

	inline void SetDepthFailOp(bool front, VkStencilOp op)
	{
		if (front)
		{
			if (m_StateInfo.depthStencilState.front.depthFailOp == op)
			{
				return;
			}
			m_StateInfo.depthStencilState.front.depthFailOp = op;
		}
		else
		{
			if (m_StateInfo.depthStencilState.back.depthFailOp == op)
			{
				return;
			}
			m_StateInfo.depthStencilState.back.depthFailOp = op;
		}
		MarkStateDirty();
	}

	inline VkStencilOp GetDepthFailOp(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.depthFailOp : m_StateInfo.depthStencilState.back.depthFailOp;
	}

	inline void SetStencilWriteMask(bool front, uint32 mask)
	{
		if (front)
		{
			if (m_StateInfo.depthStencilState.front.writeMask == mask)
			{
				return;
			}
			m_StateInfo.depthStencilState.front.writeMask = mask;
		}
		else
		{
			if (m_StateInfo.depthStencilState.back.writeMask == mask)
			{
				return;
			}
			m_StateInfo.depthStencilState.back.writeMask = mask;
		}
		MarkStateDirty();
	}

	inline uint32 GetStencilWriteMask(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.writeMask : m_StateInfo.depthStencilState.back.writeMask;
	}

	inline void SetStencilReference(bool front, uint32 ref)
	{
		if (front)
		{
			if (m_StateInfo.depthStencilState.front.reference == ref)
			{
				return;
			}
			m_StateInfo.depthStencilState.front.reference = ref;
		}
		else
		{
			if (m_StateInfo.depthStencilState.back.reference == ref)
			{
				return;
			}
			m_StateInfo.depthStencilState.back.reference = ref;
		}
		MarkStateDirty();
	}

	inline uint32 GetStencilReference(bool front)
	{
		return front ? m_StateInfo.depthStencilState.front.reference : m_StateInfo.depthStencilState.back.reference;
	}

	inline void SetMinDepthBounds(float value)
	{
		if (m_StateInfo.depthStencilState.minDepthBounds == value)
		{
			return;
		}
		m_StateInfo.depthStencilState.minDepthBounds = value;
		MarkStateDirty();
	}

	inline float GetMinDepthBounds(float value) const
	{
		return m_StateInfo.depthStencilState.minDepthBounds;
	}

	inline void SetMaxDepthBounds(float value)
	{
		if (m_StateInfo.depthStencilState.maxDepthBounds == value)
		{
			return;
		}
		m_StateInfo.depthStencilState.maxDepthBounds = value;
		MarkStateDirty();
	}

	inline float GetMaxDepthBounds() const
	{
		return m_StateInfo.depthStencilState.maxDepthBounds;
	}

	inline uint32 GetStateHash()
	{
		if (m_InvalidStateInfo) {
			m_InvalidStateInfo = false;
			m_StateInfo.GenerateHash();
		}
        return m_StateInfo.hash;
	}
    
	inline const VulkanPipelineStateInfo& GetPipelineStateInfo() const
	{
		return m_StateInfo;
	}

	inline const std::vector<UniformBufferParam>& GetUniformBufferParams() const
	{
		return m_UniformBufferParams;
	}

	void SetParam(const std::string& name, const void* data, uint32 size);

protected:

	void GenerateShaderParams();

	inline void MarkStateDirty()
	{
		m_InvalidStateInfo = true;
	}

protected:

	uint32								m_Hash;
	bool								m_InvalidStateInfo;
	VulkanPipelineStateInfo				m_StateInfo;
	std::shared_ptr<Shader>				m_Shader;

	std::vector<UniformBufferParam>		m_UniformBufferParams;

};
