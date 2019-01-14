#pragma once

#include "Common/Common.h"
#include "VulkanPlatform.h"
#include "VulkanQueue.h"
#include "VulkanRHI.h"

#include <vector>
#include <memory>
#include <map>

struct OptionalVulkanDeviceExtensions
{
    uint32 HasKHRMaintenance1 : 1;
    uint32 HasKHRMaintenance2 : 1;
    uint32 HasMirrorClampToEdge : 1;
    uint32 HasKHRExternalMemoryCapabilities : 1;
    uint32 HasKHRGetPhysicalDeviceProperties2 : 1;
    uint32 HasKHRDedicatedAllocation : 1;
    uint32 HasEXTValidationCache : 1;
    uint32 HasAMDBufferMarker : 1;
    uint32 HasNVDiagnosticCheckpoints : 1;
    uint32 HasGoogleDisplayTiming : 1;
    uint32 HasYcbcrSampler : 1;
    
    inline bool HasGPUCrashDumpExtensions() const
    {
        return HasAMDBufferMarker || HasNVDiagnosticCheckpoints;
    }
};

class VulkanDevice
{
public:
    VulkanDevice(VkPhysicalDevice physicalDevice);
    
    virtual ~VulkanDevice();
    
    bool QueryGPU(int32 deviceIndex);
    
    void InitGPU(int32 deviceIndex);
    
    void CreateDevice();
    
    void PrepareForDestroy();
    
    void Destroy();
    
    void WaitUntilIdle();
    
    bool IsFormatSupported(VkFormat format) const;
    
    const VkComponentMapping& GetFormatComponentMapping(PixelFormat format) const;
    
    void NotifyDeletedRenderTarget(VkImage image);
    
    void NotifyDeletedImage(VkImage image);
    
    void PrepareForCPURead();
    
    void SubmitCommandsAndFlushGPU();
    
    void SetupPresentQueue(VkSurfaceKHR surface);
    
    inline std::shared_ptr<VulkanQueue> GetGraphicsQueue()
    {
        return m_GfxQueue;
    }
    
    inline std::shared_ptr<VulkanQueue> GetComputeQueue()
    {
        return m_ComputeQueue;
    }
    
    inline std::shared_ptr<VulkanQueue> GetTransferQueue()
    {
        return m_TransferQueue;
    }
    
    inline std::shared_ptr<VulkanQueue> GetPresentQueue()
    {
        return m_PresentQueue;
    }
    
    inline VkPhysicalDevice GetPhysicalHandle() const
    {
        return m_PhysicalDevice;
    }
    
    inline const VkPhysicalDeviceProperties& GetDeviceProperties() const
    {
        return m_PhysicalDeviceProperties;
    }
    
    inline const VkPhysicalDeviceLimits& GetLimits() const
    {
        return m_PhysicalDeviceProperties.limits;
    }
    
    inline const VkPhysicalDeviceFeatures& GetPhysicalFeatures() const
    {
        return m_PhysicalFeatures;
    }
    
    inline bool HasUnifiedMemory() const
    {
        return true;
    }
    
    inline uint64 GetTimestampValidBitsMask() const
    {
        return m_TimestampValidBitsMask;
    }
    
    inline VkDevice GetInstanceHandle() const
    {
        return m_Device;
    }
    
    inline VkImageView GetDefaultImageView() const
    {
        return m_DefaultImageView;
    }
    
    inline const VkFormatProperties* GetFormatProperties() const
    {
        return m_FormatProperties;
    }
    
    inline const OptionalVulkanDeviceExtensions& GetOptionalExtensions() const
    {
        return m_OptionalDeviceExtensions;
    }
    
private:
    
    void MapFormatSupport(PixelFormat format, VkFormat vkFormat);
    
    void MapFormatSupport(PixelFormat format, VkFormat vkormat, int32 blockBytes);
    
    void SetComponentMapping(PixelFormat format, VkComponentSwizzle r, VkComponentSwizzle g, VkComponentSwizzle b, VkComponentSwizzle a);
    
    void GetDeviceExtensionsAndLayers(std::vector<const char*>& outDeviceExtensions, std::vector<const char*>& outDeviceLayers, bool& bOutDebugMarkers);
    
    void ParseOptionalDeviceExtensions(const std::vector<const char*>& deviceExtensions);
    
    void SetupFormats();
    
private:
    
    VkDevice m_Device;
    VkImageView m_DefaultImageView;
    VkPhysicalDevice m_PhysicalDevice;
    VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
    VkPhysicalDeviceFeatures m_PhysicalFeatures;
    std::vector<VkQueueFamilyProperties> m_QueueFamilyProps;
    
    VkFormatProperties m_FormatProperties[VK_FORMAT_RANGE_SIZE];
    std::map<VkFormat, VkFormatProperties> m_ExtensionFormatProperties;
    
    uint64 m_TimestampValidBitsMask = 0;
    
    std::shared_ptr<VulkanQueue> m_GfxQueue;
    std::shared_ptr<VulkanQueue> m_ComputeQueue;
    std::shared_ptr<VulkanQueue> m_TransferQueue;
    std::shared_ptr<VulkanQueue> m_PresentQueue;
    
    VkComponentMapping m_PixelFormatComponentMapping[PF_MAX];
    OptionalVulkanDeviceExtensions m_OptionalDeviceExtensions;
    
    friend class VulkanRHI;
};
