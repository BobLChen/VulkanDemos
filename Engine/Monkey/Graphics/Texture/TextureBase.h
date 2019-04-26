#pragma once

#include "Vulkan/VulkanPlatform.h"

class Texture2D;

class TextureBase
{
public:

	TextureBase();

	virtual ~TextureBase();

	bool IsValid() const {
		return !m_Invalid;
	}
    
    const VkDescriptorImageInfo& GetDescriptorImageInfo() const
    {
        return m_DescriptorInfo;
    }
    
protected:
	friend class Texture2D;

protected:
	VkImage						m_Image;
	VkImageLayout				m_ImageLayout;
	VkDeviceMemory				m_ImageMemory;
	VkImageView					m_ImageView;
	VkSampler					m_ImageSampler;
	VkDescriptorImageInfo		m_DescriptorInfo;

	uint32						m_Width;
	uint32						m_Height;
	uint32						m_Depth;
	uint32						m_MipLevels;
	uint32						m_LayerCount;

	bool						m_Invalid;
};
