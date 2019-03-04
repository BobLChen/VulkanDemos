#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanRHI.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Data/VertexBuffer.h"
#include <memory>

class Material
{
public:
    Material(std::shared_ptr<Shader> shader);

    virtual ~Material();

	VkPipeline GetPipeline(const VertexInputDeclareInfo& inputStateInfo, const VertexInputBindingInfo& inputBindingInfo);

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
		if (m_InputAssemblyState.topology == topology)
		{
			return;
		}
		m_InputAssemblyState.topology = topology;
		InvalidPipeline();
	}

	FORCEINLINE VkPrimitiveTopology GetTopology() const
	{
		return m_InputAssemblyState.topology;
	}

    // ----------------------------------- VkPipelineRasterizationStateCreateInfo -----------------------------------

	FORCEINLINE void SetpolygonMode(VkPolygonMode polygonMode)
	{
		if (m_RasterizationState.polygonMode == polygonMode) 
		{
			return;
		}
		m_RasterizationState.polygonMode = polygonMode;
		InvalidPipeline();
	}

	FORCEINLINE VkPolygonMode GetPolygonMode() const
	{
		return m_RasterizationState.polygonMode;
	}
    
	FORCEINLINE void SetCullMode(VkCullModeFlags cullMode)
	{
		if (m_RasterizationState.cullMode == cullMode)
		{
			return;
		}
		m_RasterizationState.cullMode = cullMode;
		InvalidPipeline();
	}

	FORCEINLINE VkCullModeFlags GetCullMode() const
	{
		return m_RasterizationState.cullMode;
	}

	FORCEINLINE void SetFrontFace(VkFrontFace frontFace)
	{
		if (m_RasterizationState.frontFace == frontFace)
		{
			return;
		}
		m_RasterizationState.frontFace = frontFace;
		InvalidPipeline();
	}

	FORCEINLINE VkFrontFace GetFrontFace() const
	{
		return m_RasterizationState.frontFace;
	}

	FORCEINLINE void SetDepthClampEnable(VkBool32 enable)
	{
		if (m_RasterizationState.depthClampEnable == enable)
		{
			return;
		}
		m_RasterizationState.depthClampEnable = enable;
		InvalidPipeline();
	}

	FORCEINLINE VkBool32 GetDepthClampEnable() const
	{
		return m_RasterizationState.depthClampEnable;
	}

	FORCEINLINE void SetDiscardEnable(VkBool32 enable)
	{
		if (m_RasterizationState.rasterizerDiscardEnable == enable)
		{
			return;
		}
		m_RasterizationState.rasterizerDiscardEnable = enable;
		InvalidPipeline();
	}

	FORCEINLINE VkBool32 GetDiscardEnable() const
	{
		return m_RasterizationState.rasterizerDiscardEnable;
	}

	FORCEINLINE void SetDepthBiasEnable(VkBool32 enable)
	{
		if (m_RasterizationState.depthBiasEnable == enable)
		{
			return;
		}
		m_RasterizationState.depthBiasEnable = enable;
		InvalidPipeline();
	}

	FORCEINLINE VkBool32 GetDepthBiasEnable() const
	{
		return m_RasterizationState.depthBiasEnable;
	}

	FORCEINLINE void SetLineWidth(float width)
	{
		if (m_RasterizationState.lineWidth == width)
		{
			return;
		}
		m_RasterizationState.lineWidth = width;
		InvalidPipeline();
	}

	FORCEINLINE float GetLineWidth() const
	{
		return m_RasterizationState.lineWidth;
	}

    // ----------------------------------- VkPipelineColorBlendAttachmentState -----------------------------------

	FORCEINLINE void SetColorWriteMask(VkColorComponentFlags mask)
	{
		if (m_ColorBlendAttachmentState.colorWriteMask == mask)
		{
			return;
		}
		m_ColorBlendAttachmentState.colorWriteMask = mask;
		InvalidPipeline();
	}

	FORCEINLINE VkColorComponentFlags GetColorWriteMask() const
	{
		return m_ColorBlendAttachmentState.colorWriteMask;
	}

	FORCEINLINE void SetBlendEnable(VkBool32 enable)
	{
		if (m_ColorBlendAttachmentState.blendEnable == enable)
		{
			return;
		}
		m_ColorBlendAttachmentState.blendEnable = enable;
		InvalidPipeline();
	}

	FORCEINLINE VkBool32 GetBlendEnable() const
	{
		return m_ColorBlendAttachmentState.blendEnable;
	}

	FORCEINLINE void SetSrcColorBlendFactor(VkBlendFactor op)
	{
		if (m_ColorBlendAttachmentState.srcColorBlendFactor == op)
		{
			return;
		}
		m_ColorBlendAttachmentState.srcColorBlendFactor = op;
		InvalidPipeline();
	}

	FORCEINLINE VkBlendFactor GetSrcColorBlendFactor() const
	{
		return m_ColorBlendAttachmentState.srcColorBlendFactor;
	}

	FORCEINLINE void SetDstColorBlendFactor(VkBlendFactor op)
	{
		if (m_ColorBlendAttachmentState.dstColorBlendFactor == op)
		{
			return;
		}
		m_ColorBlendAttachmentState.dstColorBlendFactor = op;
		InvalidPipeline();
	}

	FORCEINLINE VkBlendFactor GetDstColorBlendFactor() const
	{
		return m_ColorBlendAttachmentState.dstColorBlendFactor;
	}

	FORCEINLINE void SetColorBlendOp(VkBlendOp op)
	{
		if (m_ColorBlendAttachmentState.colorBlendOp == op)
		{
			return;
		}
		m_ColorBlendAttachmentState.colorBlendOp = op;
		InvalidPipeline();
	}

	FORCEINLINE VkBlendOp GetColorBlendOp() const
	{
		return m_ColorBlendAttachmentState.colorBlendOp;
	}

	FORCEINLINE void SetSrcAlphaBlendFactor(VkBlendFactor op)
	{
		if (m_ColorBlendAttachmentState.srcAlphaBlendFactor == op)
		{
			return;
		}
		m_ColorBlendAttachmentState.srcAlphaBlendFactor = op;
		InvalidPipeline();
	}

	FORCEINLINE VkBlendFactor GetSrcAlphaBlendFactor() const
	{
		return m_ColorBlendAttachmentState.srcAlphaBlendFactor;
	}

	FORCEINLINE void SetDstAlphaBlendFactor(VkBlendFactor op)
	{
		if (m_ColorBlendAttachmentState.dstAlphaBlendFactor == op)
		{
			return;
		}
		m_ColorBlendAttachmentState.dstAlphaBlendFactor = op;
		InvalidPipeline();
	}

	FORCEINLINE VkBlendFactor GetDstAlphaBlendFactor() const
	{
		return m_ColorBlendAttachmentState.dstAlphaBlendFactor;
	}

	FORCEINLINE void SetAlphaBlendOp(VkBlendOp op)
	{
		if (m_ColorBlendAttachmentState.alphaBlendOp == op)
		{
			return;
		}
		m_ColorBlendAttachmentState.alphaBlendOp = op;
		InvalidPipeline();
	}

	FORCEINLINE VkBlendOp GetAlphaBlendOp() const
	{
		return m_ColorBlendAttachmentState.alphaBlendOp;
	}

    // ----------------------------------- VkPipelineDepthStencilStateCreateInfo -----------------------------------

	FORCEINLINE void SetDepthTestEnable(VkBool32 enable)
	{
		if (m_DepthStencilState.depthTestEnable == enable)
		{
			return;
		}
		m_DepthStencilState.depthTestEnable = enable;
		InvalidPipeline();
	}

	FORCEINLINE VkBool32 GetDepthTestEnable() const
	{
		return m_DepthStencilState.depthTestEnable;
	}

	FORCEINLINE void SetDepthWriteEnable(VkBool32 enable)
	{
		if (m_DepthStencilState.depthWriteEnable == enable)
		{
			return;
		}
		m_DepthStencilState.depthWriteEnable = enable;
		InvalidPipeline();
	}

	FORCEINLINE VkBool32 GetDepthWriteEnable() const
	{
		return m_DepthStencilState.depthWriteEnable;
	}

	FORCEINLINE void SetDepthCompareOp(VkCompareOp op)
	{
		if (m_DepthStencilState.depthCompareOp == op)
		{
			return;
		}
		m_DepthStencilState.depthCompareOp = op;
		InvalidPipeline();
	}

	FORCEINLINE VkCompareOp GetDepthCompareOp() const
	{
		return m_DepthStencilState.depthCompareOp;
	}

	FORCEINLINE void SetDepthBoundsTestEnable(VkBool32 enable)
	{
		if (m_DepthStencilState.depthBoundsTestEnable == enable)
		{
			return;
		}
		m_DepthStencilState.depthBoundsTestEnable = enable;
		InvalidPipeline();
	}

	FORCEINLINE VkBool32 GetDepthBoundsTestEnable() const
	{
		return m_DepthStencilState.depthBoundsTestEnable;
	}

	FORCEINLINE void SetStencilTestEnable(VkBool32 enable)
	{
		if (m_DepthStencilState.depthTestEnable == enable)
		{
			return;
		}
		m_DepthStencilState.depthTestEnable = enable;
		InvalidPipeline();
	}

	FORCEINLINE VkBool32 GetStencilTestEnable() const
	{
		return m_DepthStencilState.depthTestEnable;
	}

	FORCEINLINE void SetDepthStencilCompareMask(bool front, uint32 mask)
	{
		if (front)
		{
			if (m_DepthStencilState.front.compareMask == mask)
			{
				return;
			}
			m_DepthStencilState.front.compareMask = mask;
		}
		else
		{
			if (m_DepthStencilState.back.compareMask == mask)
			{
				return;
			}
			m_DepthStencilState.back.compareMask = mask;
		}
		InvalidPipeline();
	}

	FORCEINLINE uint32 GetDepthStencilCompareMask(bool front) const
	{
		return front ? m_DepthStencilState.front.compareMask : m_DepthStencilState.back.compareMask;
	}

	FORCEINLINE void SetDepthStencilCompareOp(bool front, VkCompareOp op)
	{
		if (front)
		{
			if (m_DepthStencilState.front.compareOp == op)
			{
				return;
			}
			m_DepthStencilState.front.compareOp = op;
		}
		else
		{
			if (m_DepthStencilState.back.compareOp == op)
			{
				return;
			}
			m_DepthStencilState.back.compareOp = op;
		}
		InvalidPipeline();
	}

	FORCEINLINE VkCompareOp GetDepthStencilCompareOp(bool front) const
	{
		return front ? m_DepthStencilState.front.compareOp : m_DepthStencilState.back.compareOp;
	}

	FORCEINLINE void SetStencilFailOp(bool front, VkStencilOp op)
	{
		if (front)
		{
			if (m_DepthStencilState.front.failOp == op)
			{
				return;
			}
			m_DepthStencilState.front.failOp = op;
		}
		else
		{
			if (m_DepthStencilState.back.failOp == op)
			{
				return;
			}
			m_DepthStencilState.back.failOp = op;
		}
		InvalidPipeline();
	}

	FORCEINLINE VkStencilOp GetStencilFailOp(bool front) const
	{
		return front ? m_DepthStencilState.front.failOp : m_DepthStencilState.back.failOp;
	}

	FORCEINLINE void SetStencilPassOp(bool front, VkStencilOp op)
	{
		if (front)
		{
			if (m_DepthStencilState.front.passOp == op)
			{
				return;
			}
			m_DepthStencilState.front.passOp = op;
		}
		else
		{
			if (m_DepthStencilState.back.passOp == op)
			{
				return;
			}
			m_DepthStencilState.front.passOp = op;
		}
		InvalidPipeline();
	}

	FORCEINLINE VkStencilOp GetStencilPassOp(bool front) const
	{
		return front ? m_DepthStencilState.front.passOp : m_DepthStencilState.back.passOp;
	}

	FORCEINLINE void SetDepthFailOp(bool front, VkStencilOp op)
	{
		if (front)
		{
			if (m_DepthStencilState.front.depthFailOp == op)
			{
				return;
			}
			m_DepthStencilState.front.depthFailOp = op;
		}
		else
		{
			if (m_DepthStencilState.back.depthFailOp == op)
			{
				return;
			}
			m_DepthStencilState.back.depthFailOp = op;
		}
		InvalidPipeline();
	}

	FORCEINLINE VkStencilOp GetDepthFailOp(bool front) const
	{
		return front ? m_DepthStencilState.front.depthFailOp : m_DepthStencilState.back.depthFailOp;
	}

	FORCEINLINE void SetStencilWriteMask(bool front, uint32 mask)
	{
		if (front)
		{
			if (m_DepthStencilState.front.writeMask == mask)
			{
				return;
			}
			m_DepthStencilState.front.writeMask = mask;
		}
		else
		{
			if (m_DepthStencilState.back.writeMask == mask)
			{
				return;
			}
			m_DepthStencilState.back.writeMask = mask;
		}
		InvalidPipeline();
	}

	FORCEINLINE uint32 GetStencilWriteMask(bool front) const
	{
		return front ? m_DepthStencilState.front.writeMask : m_DepthStencilState.back.writeMask;
	}

	FORCEINLINE void SetStencilReference(bool front, uint32 ref)
	{
		if (front)
		{
			if (m_DepthStencilState.front.reference == ref)
			{
				return;
			}
			m_DepthStencilState.front.reference = ref;
		}
		else
		{
			if (m_DepthStencilState.back.reference == ref)
			{
				return;
			}
			m_DepthStencilState.back.reference = ref;
		}
		InvalidPipeline();
	}

	FORCEINLINE uint32 GetStencilReference(bool front)
	{
		return front ? m_DepthStencilState.front.reference : m_DepthStencilState.back.reference;
	}

	FORCEINLINE void SetMinDepthBounds(float value)
	{
		if (m_DepthStencilState.minDepthBounds == value)
		{
			return;
		}
		m_DepthStencilState.minDepthBounds = value;
		InvalidPipeline();
	}

	FORCEINLINE float GetMinDepthBounds(float value) const
	{
		return m_DepthStencilState.minDepthBounds;
	}

	FORCEINLINE void SetMaxDepthBounds(float value)
	{
		if (m_DepthStencilState.maxDepthBounds == value)
		{
			return;
		}
		m_DepthStencilState.maxDepthBounds = value;
		InvalidPipeline();
	}

	FORCEINLINE float GetMaxDepthBounds() const
	{
		return m_DepthStencilState.maxDepthBounds;
	}

protected:

	virtual void InitState();

	FORCEINLINE void InvalidPipeline()
	{
		m_InvalidPipeline = true;
	}

private:

	VkPipelineInputAssemblyStateCreateInfo	m_InputAssemblyState;
	VkPipelineRasterizationStateCreateInfo	m_RasterizationState;
	VkPipelineColorBlendAttachmentState		m_ColorBlendAttachmentState;
	VkPipelineViewportStateCreateInfo		m_ViewportState;
	VkPipelineDepthStencilStateCreateInfo	m_DepthStencilState;
	VkPipelineMultisampleStateCreateInfo	m_MultisampleState;

protected:

	uint32 m_Hash;
	bool   m_InvalidPipeline;
	std::shared_ptr<Shader> m_Shader;
};


