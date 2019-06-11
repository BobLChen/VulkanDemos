#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanPipeline.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Data/VertexBuffer.h"
#include <memory>

class Material
{
public:
    Material(std::shared_ptr<Shader> shader);

    virtual ~Material();
    
	FORCEINLINE void SetShader(std::shared_ptr<Shader> shader)
	{
		m_Shader = shader;
	}

	FORCEINLINE std::shared_ptr<Shader> GetShader() const
	{
		return m_Shader;
	}

    // ----------------------------------- VkPipelineInputAssemblyStateCreateInfo -----------------------------------

	FORCEINLINE void SetTopology(VkPrimitiveTopology topology)
	{
		if (m_StateInfo.inputAssemblyState.topology == topology)
		{
			return;
		}
		m_StateInfo.inputAssemblyState.topology = topology;
		MarkStateDirty();
	}

	FORCEINLINE VkPrimitiveTopology GetTopology() const
	{
		return m_StateInfo.inputAssemblyState.topology;
	}

    // ----------------------------------- VkPipelineRasterizationStateCreateInfo -----------------------------------

	FORCEINLINE void SetpolygonMode(VkPolygonMode polygonMode)
	{
		if (m_StateInfo.rasterizationState.polygonMode == polygonMode) 
		{
			return;
		}
		m_StateInfo.rasterizationState.polygonMode = polygonMode;
		MarkStateDirty();
	}

	FORCEINLINE VkPolygonMode GetPolygonMode() const
	{
		return m_StateInfo.rasterizationState.polygonMode;
	}
    
	FORCEINLINE void SetCullMode(VkCullModeFlags cullMode)
	{
		if (m_StateInfo.rasterizationState.cullMode == cullMode)
		{
			return;
		}
		m_StateInfo.rasterizationState.cullMode = cullMode;
		MarkStateDirty();
	}

	FORCEINLINE VkCullModeFlags GetCullMode() const
	{
		return m_StateInfo.rasterizationState.cullMode;
	}

	FORCEINLINE void SetFrontFace(VkFrontFace frontFace)
	{
		if (m_StateInfo.rasterizationState.frontFace == frontFace)
		{
			return;
		}
		m_StateInfo.rasterizationState.frontFace = frontFace;
		MarkStateDirty();
	}

	FORCEINLINE VkFrontFace GetFrontFace() const
	{
		return m_StateInfo.rasterizationState.frontFace;
	}

	FORCEINLINE void SetDepthClampEnable(VkBool32 enable)
	{
		if (m_StateInfo.rasterizationState.depthClampEnable == enable)
		{
			return;
		}
		m_StateInfo.rasterizationState.depthClampEnable = enable;
		MarkStateDirty();
	}

	FORCEINLINE VkBool32 GetDepthClampEnable() const
	{
		return m_StateInfo.rasterizationState.depthClampEnable;
	}

	FORCEINLINE void SetDiscardEnable(VkBool32 enable)
	{
		if (m_StateInfo.rasterizationState.rasterizerDiscardEnable == enable)
		{
			return;
		}
		m_StateInfo.rasterizationState.rasterizerDiscardEnable = enable;
		MarkStateDirty();
	}

	FORCEINLINE VkBool32 GetDiscardEnable() const
	{
		return m_StateInfo.rasterizationState.rasterizerDiscardEnable;
	}

	FORCEINLINE void SetDepthBiasEnable(VkBool32 enable)
	{
		if (m_StateInfo.rasterizationState.depthBiasEnable == enable)
		{
			return;
		}
		m_StateInfo.rasterizationState.depthBiasEnable = enable;
		MarkStateDirty();
	}

	FORCEINLINE VkBool32 GetDepthBiasEnable() const
	{
		return m_StateInfo.rasterizationState.depthBiasEnable;
	}

	FORCEINLINE void SetLineWidth(float width)
	{
		if (m_StateInfo.rasterizationState.lineWidth == width)
		{
			return;
		}
		m_StateInfo.rasterizationState.lineWidth = width;
		MarkStateDirty();
	}

	FORCEINLINE float GetLineWidth() const
	{
		return m_StateInfo.rasterizationState.lineWidth;
	}

    // ----------------------------------- VkPipelineColorBlendAttachmentState -----------------------------------

	FORCEINLINE void SetColorWriteMask(VkColorComponentFlags mask)
	{
		if (m_StateInfo.colorBlendAttachmentState.colorWriteMask == mask)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.colorWriteMask = mask;
		MarkStateDirty();
	}

	FORCEINLINE VkColorComponentFlags GetColorWriteMask() const
	{
		return m_StateInfo.colorBlendAttachmentState.colorWriteMask;
	}

	FORCEINLINE void SetBlendEnable(VkBool32 enable)
	{
		if (m_StateInfo.colorBlendAttachmentState.blendEnable == enable)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.blendEnable = enable;
		MarkStateDirty();
	}

	FORCEINLINE VkBool32 GetBlendEnable() const
	{
		return m_StateInfo.colorBlendAttachmentState.blendEnable;
	}

	FORCEINLINE void SetSrcColorBlendFactor(VkBlendFactor op)
	{
		if (m_StateInfo.colorBlendAttachmentState.srcColorBlendFactor == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.srcColorBlendFactor = op;
		MarkStateDirty();
	}

	FORCEINLINE VkBlendFactor GetSrcColorBlendFactor() const
	{
		return m_StateInfo.colorBlendAttachmentState.srcColorBlendFactor;
	}

	FORCEINLINE void SetDstColorBlendFactor(VkBlendFactor op)
	{
		if (m_StateInfo.colorBlendAttachmentState.dstColorBlendFactor == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.dstColorBlendFactor = op;
		MarkStateDirty();
	}

	FORCEINLINE VkBlendFactor GetDstColorBlendFactor() const
	{
		return m_StateInfo.colorBlendAttachmentState.dstColorBlendFactor;
	}

	FORCEINLINE void SetColorBlendOp(VkBlendOp op)
	{
		if (m_StateInfo.colorBlendAttachmentState.colorBlendOp == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.colorBlendOp = op;
		MarkStateDirty();
	}

	FORCEINLINE VkBlendOp GetColorBlendOp() const
	{
		return m_StateInfo.colorBlendAttachmentState.colorBlendOp;
	}

	FORCEINLINE void SetSrcAlphaBlendFactor(VkBlendFactor op)
	{
		if (m_StateInfo.colorBlendAttachmentState.srcAlphaBlendFactor == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.srcAlphaBlendFactor = op;
		MarkStateDirty();
	}

	FORCEINLINE VkBlendFactor GetSrcAlphaBlendFactor() const
	{
		return m_StateInfo.colorBlendAttachmentState.srcAlphaBlendFactor;
	}

	FORCEINLINE void SetDstAlphaBlendFactor(VkBlendFactor op)
	{
		if (m_StateInfo.colorBlendAttachmentState.dstAlphaBlendFactor == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.dstAlphaBlendFactor = op;
		MarkStateDirty();
	}

	FORCEINLINE VkBlendFactor GetDstAlphaBlendFactor() const
	{
		return m_StateInfo.colorBlendAttachmentState.dstAlphaBlendFactor;
	}

	FORCEINLINE void SetAlphaBlendOp(VkBlendOp op)
	{
		if (m_StateInfo.colorBlendAttachmentState.alphaBlendOp == op)
		{
			return;
		}
		m_StateInfo.colorBlendAttachmentState.alphaBlendOp = op;
		MarkStateDirty();
	}

	FORCEINLINE VkBlendOp GetAlphaBlendOp() const
	{
		return m_StateInfo.colorBlendAttachmentState.alphaBlendOp;
	}

    // ----------------------------------- VkPipelineDepthStencilStateCreateInfo -----------------------------------

	FORCEINLINE void SetDepthTestEnable(VkBool32 enable)
	{
		if (m_StateInfo.depthStencilState.depthTestEnable == enable)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthTestEnable = enable;
		MarkStateDirty();
	}

	FORCEINLINE VkBool32 GetDepthTestEnable() const
	{
		return m_StateInfo.depthStencilState.depthTestEnable;
	}

	FORCEINLINE void SetDepthWriteEnable(VkBool32 enable)
	{
		if (m_StateInfo.depthStencilState.depthWriteEnable == enable)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthWriteEnable = enable;
		MarkStateDirty();
	}

	FORCEINLINE VkBool32 GetDepthWriteEnable() const
	{
		return m_StateInfo.depthStencilState.depthWriteEnable;
	}

	FORCEINLINE void SetDepthCompareOp(VkCompareOp op)
	{
		if (m_StateInfo.depthStencilState.depthCompareOp == op)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthCompareOp = op;
		MarkStateDirty();
	}

	FORCEINLINE VkCompareOp GetDepthCompareOp() const
	{
		return m_StateInfo.depthStencilState.depthCompareOp;
	}

	FORCEINLINE void SetDepthBoundsTestEnable(VkBool32 enable)
	{
		if (m_StateInfo.depthStencilState.depthBoundsTestEnable == enable)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthBoundsTestEnable = enable;
		MarkStateDirty();
	}

	FORCEINLINE VkBool32 GetDepthBoundsTestEnable() const
	{
		return m_StateInfo.depthStencilState.depthBoundsTestEnable;
	}

	FORCEINLINE void SetStencilTestEnable(VkBool32 enable)
	{
		if (m_StateInfo.depthStencilState.depthTestEnable == enable)
		{
			return;
		}
		m_StateInfo.depthStencilState.depthTestEnable = enable;
		MarkStateDirty();
	}

	FORCEINLINE VkBool32 GetStencilTestEnable() const
	{
		return m_StateInfo.depthStencilState.depthTestEnable;
	}

	FORCEINLINE void SetDepthStencilCompareMask(bool front, uint32 mask)
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

	FORCEINLINE uint32 GetDepthStencilCompareMask(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.compareMask : m_StateInfo.depthStencilState.back.compareMask;
	}

	FORCEINLINE void SetDepthStencilCompareOp(bool front, VkCompareOp op)
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

	FORCEINLINE VkCompareOp GetDepthStencilCompareOp(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.compareOp : m_StateInfo.depthStencilState.back.compareOp;
	}

	FORCEINLINE void SetStencilFailOp(bool front, VkStencilOp op)
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

	FORCEINLINE VkStencilOp GetStencilFailOp(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.failOp : m_StateInfo.depthStencilState.back.failOp;
	}

	FORCEINLINE void SetStencilPassOp(bool front, VkStencilOp op)
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

	FORCEINLINE VkStencilOp GetStencilPassOp(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.passOp : m_StateInfo.depthStencilState.back.passOp;
	}

	FORCEINLINE void SetDepthFailOp(bool front, VkStencilOp op)
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

	FORCEINLINE VkStencilOp GetDepthFailOp(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.depthFailOp : m_StateInfo.depthStencilState.back.depthFailOp;
	}

	FORCEINLINE void SetStencilWriteMask(bool front, uint32 mask)
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

	FORCEINLINE uint32 GetStencilWriteMask(bool front) const
	{
		return front ? m_StateInfo.depthStencilState.front.writeMask : m_StateInfo.depthStencilState.back.writeMask;
	}

	FORCEINLINE void SetStencilReference(bool front, uint32 ref)
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

	FORCEINLINE uint32 GetStencilReference(bool front)
	{
		return front ? m_StateInfo.depthStencilState.front.reference : m_StateInfo.depthStencilState.back.reference;
	}

	FORCEINLINE void SetMinDepthBounds(float value)
	{
		if (m_StateInfo.depthStencilState.minDepthBounds == value)
		{
			return;
		}
		m_StateInfo.depthStencilState.minDepthBounds = value;
		MarkStateDirty();
	}

	FORCEINLINE float GetMinDepthBounds(float value) const
	{
		return m_StateInfo.depthStencilState.minDepthBounds;
	}

	FORCEINLINE void SetMaxDepthBounds(float value)
	{
		if (m_StateInfo.depthStencilState.maxDepthBounds == value)
		{
			return;
		}
		m_StateInfo.depthStencilState.maxDepthBounds = value;
		MarkStateDirty();
	}

	FORCEINLINE float GetMaxDepthBounds() const
	{
		return m_StateInfo.depthStencilState.maxDepthBounds;
	}

	FORCEINLINE uint32 GetStateHash()
	{
		if (m_InvalidStateInfo) {
			m_InvalidStateInfo = false;
			m_StateInfo.GenerateHash();
		}
	}

protected:

	FORCEINLINE void MarkStateDirty()
	{
		m_InvalidStateInfo = true;
	}

protected:
	uint32					m_Hash;
	bool					m_InvalidStateInfo;
	VulkanPipelineStateInfo m_StateInfo;
	std::shared_ptr<Shader> m_Shader;
};