#include "Engine.h"
#include "TextureBase.h"

#include "Vulkan/VulkanDevice.h"

TextureBase::TextureBase()
	: m_Image(VK_NULL_HANDLE)
	, m_ImageLayout(VK_IMAGE_LAYOUT_UNDEFINED)
	, m_ImageMemory(VK_NULL_HANDLE)
	, m_ImageView(VK_NULL_HANDLE)
	, m_ImageSampler(VK_NULL_HANDLE)
	, m_Width(0)
	, m_Height(0)
	, m_Depth(0)
	, m_MipLevels(0)
	, m_LayerCount(0)
	, m_Format(VK_FORMAT_R8G8B8A8_UNORM)
	, m_Invalid(true)
{

}

TextureBase::~TextureBase()
{
	if (m_Invalid) {
		return;
	}
	m_Invalid = true;

	VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();

	if (m_ImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(device, m_ImageView, VULKAN_CPU_ALLOCATOR);
		m_ImageView = VK_NULL_HANDLE;
	}
	
	if (m_Image != VK_NULL_HANDLE) {
		vkDestroyImage(device, m_Image, VULKAN_CPU_ALLOCATOR);
		m_Image = VK_NULL_HANDLE;
	}
	
	if (m_ImageSampler != VK_NULL_HANDLE) {
		vkDestroySampler(device, m_ImageSampler, VULKAN_CPU_ALLOCATOR);
		m_ImageSampler = VK_NULL_HANDLE;
	}
	
	if (m_ImageMemory != VK_NULL_HANDLE) {
		vkFreeMemory(device, m_ImageMemory, VULKAN_CPU_ALLOCATOR);
		m_ImageMemory = VK_NULL_HANDLE;
	}
}
