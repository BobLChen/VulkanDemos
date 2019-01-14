#include "VulkanDevice.h"
#include "VulkanPlatform.h"

VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice)
    : m_Device(VK_NULL_HANDLE)
    , m_DefaultImageView(VK_NULL_HANDLE)
    , m_PhysicalDevice(physicalDevice)
    , m_GfxQueue(nullptr)
    , m_ComputeQueue(nullptr)
    , m_TransferQueue(nullptr)
    , m_PresentQueue(nullptr)
{
    
}

VulkanDevice::~VulkanDevice()
{
    if (m_Device != VK_NULL_HANDLE)
    {
        Destroy();
        m_Device = VK_NULL_HANDLE;
    }
}

void VulkanDevice::CreateDevice()
{
    
}

void VulkanDevice::SetupFormats()
{
    
}

void VulkanDevice::MapFormatSupport(PixelFormat format, VkFormat vkFormat)
{
    
}

void VulkanDevice::SetComponentMapping(PixelFormat format, VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a)
{
    
}

void VulkanDevice::MapFormatSupport(PixelFormat format, VkFormat vkFormat, int32 blockBytes)
{
    
}

bool VulkanDevice::QueryGPU(int32 deviceIndex)
{
    return true;
}

void VulkanDevice::InitGPU(int32 deviceIndex)
{
    
}

void VulkanDevice::PrepareForDestroy()
{
    
}

void VulkanDevice::Destroy()
{

}

void VulkanDevice::WaitUntilIdle()
{
    
}

bool VulkanDevice::IsFormatSupported(VkFormat format) const
{
    return true;
}

const VkComponentMapping& VulkanDevice::GetFormatComponentMapping(PixelFormat format) const
{
    VkComponentMapping mapping;
    return mapping;
}

void VulkanDevice::NotifyDeletedRenderTarget(VkImage image)
{
    
}

void VulkanDevice::NotifyDeletedImage(VkImage image)
{
    
}

void VulkanDevice::PrepareForCPURead()
{
    
}
