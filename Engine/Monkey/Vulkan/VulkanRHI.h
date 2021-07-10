#pragma once

#include "Common/Common.h"

#include "VulkanPlatform.h"
#include "VulkanGlobals.h"
#include "RHIDefinitions.h"

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

	FORCE_INLINE const std::vector<const char*>& GetInstanceExtensions() const
	{
		return m_InstanceExtensions;
	}

	FORCE_INLINE const std::vector<const char*>& GetInstanceLayers() const
	{
		return m_InstanceLayers;
	}

	FORCE_INLINE const VkInstance& GetInstance() const
	{
		return m_Instance;
	}

	FORCE_INLINE std::shared_ptr<VulkanDevice> GetDevice() const
	{
		return m_Device;
	}

	FORCE_INLINE std::shared_ptr<VulkanSwapChain> GetSwapChain() const
	{
		return m_SwapChain;
	}

	FORCE_INLINE const std::vector<VkImage>& GetBackbufferImages() const
	{
		return m_BackbufferImages;
	}

	FORCE_INLINE const std::vector<VkImageView>& GetBackbufferViews() const
	{
		return m_BackbufferViews;
	}

	FORCE_INLINE const PixelFormat& GetPixelFormat() const
	{
		return m_PixelFormat;
	}

	FORCE_INLINE const char* GetName()
	{ 
		return "Vulkan";
	}

	void AddAppDeviceExtensions(const char* name)
	{
		m_AppDeviceExtensions.push_back(name);
	}

	void AddAppInstanceExtensions(const char* name)
	{
		m_AppInstanceExtensions.push_back(name);
	}

	FORCE_INLINE void SetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures2* deviceFeatures)
	{
		m_PhysicalDeviceFeatures2 = deviceFeatures;
	}

protected:

	void CreateInstance();

	void SelectAndInitDevice();

	static void GetInstanceLayersAndExtensions(std::vector<const char*>& outInstanceExtensions, std::vector<const char*>& outInstanceLayers);

#if MONKEY_DEBUG
    void SetupDebugLayerCallback();

    void RemoveDebugLayerCallback();
#endif

	void InitInstance();

	void RecreateSwapChain();

	void DestorySwapChain();

protected:

#if MONKEY_DEBUG
	VkDebugUtilsMessengerEXT			m_MsgCallback = VK_NULL_HANDLE;
#endif
    
	VkInstance							m_Instance;
    std::vector<const char*>			m_InstanceLayers;
    std::vector<const char*>			m_InstanceExtensions;
	std::vector<const char*>			m_AppDeviceExtensions;
	std::vector<const char*>			m_AppInstanceExtensions;
	VkPhysicalDeviceFeatures2*			m_PhysicalDeviceFeatures2 = nullptr;

    std::shared_ptr<VulkanDevice>		m_Device;

    std::shared_ptr<VulkanSwapChain>	m_SwapChain;
	PixelFormat							m_PixelFormat;
	std::vector<VkImage>				m_BackbufferImages;
	std::vector<VkImageView>			m_BackbufferViews;
};


FORCE_INLINE VkFormat PixelFormatToVkFormat(PixelFormat format, const bool bIsSRGB)
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

static FORCE_INLINE VkFormat VEToVkFormat(VertexElementType Type)
{
	switch (Type)
	{
	case VET_Float1:
		return VK_FORMAT_R32_SFLOAT;
	case VET_Float2:
		return VK_FORMAT_R32G32_SFLOAT;
	case VET_Float3:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case VET_PackedNormal:
		return VK_FORMAT_R8G8B8A8_SNORM;
	case VET_UByte4:
		return VK_FORMAT_R8G8B8A8_UINT;
	case VET_UByte4N:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case VET_Color:
		return VK_FORMAT_B8G8R8A8_UNORM;
	case VET_Short2:
		return VK_FORMAT_R16G16_SINT;
	case VET_Short4:
		return VK_FORMAT_R16G16B16A16_SINT;
	case VET_Short2N:
		return VK_FORMAT_R16G16_SNORM;
	case VET_Half2:
		return VK_FORMAT_R16G16_SFLOAT;
	case VET_Half4:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case VET_Short4N:
		return VK_FORMAT_R16G16B16A16_SNORM;
	case VET_UShort2:
		return VK_FORMAT_R16G16_UINT;
	case VET_UShort4:
		return VK_FORMAT_R16G16B16A16_UINT;
	case VET_UShort2N:
		return VK_FORMAT_R16G16_UNORM;
	case VET_UShort4N:
		return VK_FORMAT_R16G16B16A16_UNORM;
	case VET_Float4:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case VET_URGB10A2N:
		return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	default:
		break;
	}

	return VK_FORMAT_UNDEFINED;
}

static FORCE_INLINE uint32 ElementTypeToSize(VertexElementType type)
{
	switch (type)
	{
	case VET_Float1:
		return 4;
	case VET_Float2:
		return 8;
	case VET_Float3:
		return 12;
	case VET_Float4:
		return 16;
	case VET_PackedNormal:
		return 4;
	case VET_UByte4:
		return 4;
	case VET_UByte4N:
		return 4;
	case VET_Color:
		return 4;
	case VET_Short2:
		return 4;
	case VET_Short4:
		return 8;
	case VET_UShort2:
		return 4;
	case VET_UShort4:
		return 8;
	case VET_Short2N:
		return 4;
	case VET_UShort2N:
		return 4;
	case VET_Half2:
		return 4;
	case VET_Half4:
		return 8;
	case VET_Short4N:
		return 8;
	case VET_UShort4N:
		return 8;
	case VET_URGB10A2N:
		return 4;
	default:
		return 0;
	};
}

static FORCE_INLINE uint32 IndexTypeToSize(VkIndexType type)
{
	switch (type)
	{
	case VK_INDEX_TYPE_UINT16:
		return 2;
		break;
	case VK_INDEX_TYPE_UINT32:
		return 4;
		break;
    default:
        return 0;
        break;
    }
}
