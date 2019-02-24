#pragma once

#include "Common/Common.h"
#include "VulkanPlatform.h"
#include "VulkanGlobals.h"
#include <string>

class VulkanDevice;
class VulkanQueue;
class VulkanSwapChain;

class VulkanRHI
{
public:
	VulkanRHI();

	virtual ~VulkanRHI();

	void Init();

	void PostInit();

	void Shutdown();;

	inline const std::vector<const char*>& GetInstanceExtensions() const
	{
		return m_InstanceExtensions;
	}

	inline const std::vector<const char*>& GetInstanceLayers() const
	{
		return m_InstanceLayers;
	}

	inline VkInstance GetInstance() const
	{
		return m_Instance;
	}

	inline std::shared_ptr<VulkanDevice> GetDevice()
	{
		return m_Device;
	}
    
	inline VkSemaphore GetPresentCompleteSemaphore()
	{
		return m_PresentComplete;
	}

	inline VkSemaphore GetRenderCompleteSemaphore()
	{
		return m_RenderComplete;
	}

	inline VkSubmitInfo GetSumbitInfo()
	{
		return m_SubmitInfo;
	}

	inline VkCommandPool GetCommandPool()
	{
		return m_CommandPool;
	}

	inline std::vector<VkCommandBuffer>& GetCommandBuffers()
	{
		return m_CommandBuffers;
	}

	inline std::shared_ptr<VulkanSwapChain> GetSwapChain()
	{
		return m_SwapChain;
	}

	inline std::vector<VkImage>& GetFrameImages()
	{
		return m_FrameImages;
	}

	inline std::vector<VkImageView>& GetFrameImageViews()
	{
		return m_FrameImageViews;
	}

	inline std::vector<VkFramebuffer>& GetFrameBuffers()
	{
		return m_FrameBuffers;
	}

	inline VkImage GetDepthStencilImage()
	{
		return m_DepthStencilImage;
	}

	inline VkImageView GetDepthStencilView()
	{
		return m_DepthStencilView;
	}

	inline VkDeviceMemory GetDepthStencilMemory()
	{
		return m_DepthStencilMemory;
	}

	inline PixelFormat GetPixelFormat()
	{
		return m_PixelFormat;
	}

	inline PixelFormat GetDepthFormat()
	{
		return m_DepthFormat;
	}

	inline VkRenderPass GetRenderPass()
	{
		return m_RenderPass;
	}

	inline VkPipelineCache GetPipelineCache()
	{
		return m_PipelineCache;
	}

	inline bool SupportsDebugUtilsExt() const
	{
		return m_SupportsDebugUtilsExt;
	}
	
    inline VkSampleCountFlagBits GetSampleCount() const
    {
        return m_SampleCount;
    }
    
	inline const char* GetName()
	{ 
		return "Vulkan";
	}

protected:
	void CreateInstance();

	void SelectAndInitDevice();

	static void GetInstanceLayersAndExtensions(std::vector<const char*>& outInstanceExtensions, std::vector<const char*>& outInstanceLayers, bool& outDebugUtils);

#if MONKEY_DEBUG
    VkDebugReportCallbackEXT m_MsgCallback = VK_NULL_HANDLE;

    void SetupDebugLayerCallback();

    void RemoveDebugLayerCallback();
#endif

	void InitInstance();

	void CreateSemaphores();

	void DestorySemaphore();

	void RecreateSwapChain();

	void DestorySwapChain();

	void CreateCommandPool();

	void DestoryCommandPool();

	void CreateCommandBuffers();

	void DestoryCommandBuffers();

	void CreateDepthStencil();

	void DestoryDepthStencil();

	void CreateRenderPass();

	void DestoryRenderPass();

	void CreatePipelineCache();

	void DestoryPipelineCache();

	void CreateFrameBuffer();

	void DestroyFrameBuffer();
    
protected:
    
	VkInstance m_Instance;
    std::vector<const char*> m_InstanceLayers;
    std::vector<const char*> m_InstanceExtensions;
    
    VkSemaphore m_PresentComplete;
	VkSemaphore m_RenderComplete;
	
    VkSubmitInfo m_SubmitInfo;
	
    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    
	VkPipelineStageFlags m_SubmitPipelineStages;
    
    std::shared_ptr<VulkanDevice> m_Device;
    std::vector<std::shared_ptr<VulkanDevice>> m_Devices;
    
    std::shared_ptr<VulkanSwapChain> m_SwapChain;
    std::vector<VkImage> m_FrameImages;
    std::vector<VkImageView> m_FrameImageViews;
	std::vector<VkFramebuffer> m_FrameBuffers;
    
    VkImage m_DepthStencilImage;
    VkImageView m_DepthStencilView;
    VkDeviceMemory m_DepthStencilMemory;
    PixelFormat m_PixelFormat;
    PixelFormat m_DepthFormat;
    VkRenderPass m_RenderPass;
    VkPipelineCache m_PipelineCache;
    VkSampleCountFlagBits m_SampleCount;
    
	bool m_SupportsDebugUtilsExt;
};


inline VkFormat PixelFormatToVkFormat(PixelFormat format, const bool bIsSRGB)
{
	VkFormat result = (VkFormat)G_PixelFormats[format].platformFormat;
	if (bIsSRGB)
	{
		switch (result)
		{
		case VK_FORMAT_B8G8R8A8_UNORM:				result = VK_FORMAT_B8G8R8A8_SRGB; break;
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:		result = VK_FORMAT_A8B8G8R8_SRGB_PACK32; break;
		case VK_FORMAT_R8_UNORM:					result = VK_FORMAT_R8_SRGB; break;
		case VK_FORMAT_R8G8_UNORM:					result = VK_FORMAT_R8G8_SRGB; break;
		case VK_FORMAT_R8G8B8_UNORM:				result = VK_FORMAT_R8G8B8_SRGB; break;
		case VK_FORMAT_R8G8B8A8_UNORM:				result = VK_FORMAT_R8G8B8A8_SRGB; break;
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:			result = VK_FORMAT_BC1_RGB_SRGB_BLOCK; break;
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:		result = VK_FORMAT_BC1_RGBA_SRGB_BLOCK; break;
		case VK_FORMAT_BC2_UNORM_BLOCK:				result = VK_FORMAT_BC2_SRGB_BLOCK; break;
		case VK_FORMAT_BC3_UNORM_BLOCK:				result = VK_FORMAT_BC3_SRGB_BLOCK; break;
		case VK_FORMAT_BC7_UNORM_BLOCK:				result = VK_FORMAT_BC7_SRGB_BLOCK; break;
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:		result = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK; break;
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:	result = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK; break;
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:	result = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:		result = VK_FORMAT_ASTC_4x4_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:		result = VK_FORMAT_ASTC_5x4_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:		result = VK_FORMAT_ASTC_5x5_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:		result = VK_FORMAT_ASTC_6x5_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:		result = VK_FORMAT_ASTC_6x6_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:		result = VK_FORMAT_ASTC_8x5_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:		result = VK_FORMAT_ASTC_8x6_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:		result = VK_FORMAT_ASTC_8x8_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:		result = VK_FORMAT_ASTC_10x5_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:		result = VK_FORMAT_ASTC_10x6_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:		result = VK_FORMAT_ASTC_10x8_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:		result = VK_FORMAT_ASTC_10x10_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:		result = VK_FORMAT_ASTC_12x10_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:		result = VK_FORMAT_ASTC_12x12_SRGB_BLOCK; break;
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:	result = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG; break;
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:	result = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG; break;
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:	result = VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG; break;
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:	result = VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG; break;
		default:	break;
		}
	}
	return result;
}



