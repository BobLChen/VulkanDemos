#pragma once

#include "Common/Common.h"

#include "VulkanPlatform.h"
#include "VulkanQueue.h"
#include "VulkanMemory.h"
#include "VulkanRHI.h"

#include <vector>
#include <memory>
#include <map>

class VulkanFenceManager;
class VulkanDeviceMemoryManager;

class VulkanDevice
{
public:
    VulkanDevice(VkPhysicalDevice physicalDevice);
    
    virtual ~VulkanDevice();
    
    bool QueryGPU(int32 deviceIndex);
    
    void InitGPU(int32 deviceIndex);
    
    void CreateDevice();
    
    void Destroy();
    
    bool IsFormatSupported(VkFormat format);
    
    const VkComponentMapping& GetFormatComponentMapping(PixelFormat format) const;
    
    void SetupPresentQueue(VkSurfaceKHR surface);
    
    FORCE_INLINE std::shared_ptr<VulkanQueue> GetGraphicsQueue()
    {
        return m_GfxQueue;
    }
    
    FORCE_INLINE std::shared_ptr<VulkanQueue> GetComputeQueue()
    {
        return m_ComputeQueue;
    }
    
    FORCE_INLINE std::shared_ptr<VulkanQueue> GetTransferQueue()
    {
        return m_TransferQueue;
    }
    
    FORCE_INLINE std::shared_ptr<VulkanQueue> GetPresentQueue()
    {
        return m_PresentQueue;
    }
    
    FORCE_INLINE VkPhysicalDevice GetPhysicalHandle() const
    {
        return m_PhysicalDevice;
    }
    
    FORCE_INLINE const VkPhysicalDeviceProperties& GetDeviceProperties() const
    {
        return m_PhysicalDeviceProperties;
    }
    
    FORCE_INLINE const VkPhysicalDeviceLimits& GetLimits() const
    {
        return m_PhysicalDeviceProperties.limits;
    }
    
    FORCE_INLINE const VkPhysicalDeviceFeatures& GetPhysicalFeatures() const
    {
        return m_PhysicalDeviceFeatures;
    }
    
    FORCE_INLINE VkDevice GetInstanceHandle() const
    {
        return m_Device;
    }
    
    FORCE_INLINE const VkFormatProperties* GetFormatProperties() const
    {
        return m_FormatProperties;
    }
    
	FORCE_INLINE VulkanFenceManager& GetFenceManager()
	{
		return *m_FenceManager;
	}
    
    FORCE_INLINE VulkanDeviceMemoryManager& GetMemoryManager()
    {
        return *m_MemoryManager;
    }
    
	FORCE_INLINE void AddAppDeviceExtensions(const char* name)
	{
		m_AppDeviceExtensions.push_back(name);
	}

	FORCE_INLINE void SetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures2* deviceFeatures)
	{
		m_PhysicalDeviceFeatures2 = deviceFeatures;
	}

private:
    
    void MapFormatSupport(PixelFormat format, VkFormat vkFormat);
    
    void MapFormatSupport(PixelFormat format, VkFormat vkormat, int32 blockBytes);
    
    void SetComponentMapping(PixelFormat format, VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a);
    
    void GetDeviceExtensionsAndLayers(std::vector<const char*>& outDeviceExtensions, std::vector<const char*>& outDeviceLayers, bool& bOutDebugMarkers);
    
    void SetupFormats();
    
private:
    friend class VulkanRHI;

private:
    VkDevice                                m_Device;
    VkPhysicalDevice                        m_PhysicalDevice;
    VkPhysicalDeviceProperties              m_PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures                m_PhysicalDeviceFeatures;
    std::vector<VkQueueFamilyProperties>    m_QueueFamilyProps;
    
    VkFormatProperties                      m_FormatProperties[PF_MAX];
    std::map<VkFormat, VkFormatProperties>  m_ExtensionFormatProperties;
	VkComponentMapping                      m_PixelFormatComponentMapping[PF_MAX];

    std::shared_ptr<VulkanQueue>            m_GfxQueue;
    std::shared_ptr<VulkanQueue>            m_ComputeQueue;
    std::shared_ptr<VulkanQueue>            m_TransferQueue;
    std::shared_ptr<VulkanQueue>            m_PresentQueue;

    VulkanFenceManager*                     m_FenceManager;
    VulkanDeviceMemoryManager*              m_MemoryManager;

	std::vector<const char*>				m_AppDeviceExtensions;
	VkPhysicalDeviceFeatures2*				m_PhysicalDeviceFeatures2;
};
