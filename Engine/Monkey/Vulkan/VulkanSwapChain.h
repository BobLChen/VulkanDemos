#include "VulkanPlatform.h"
#include "Common/Common.h"

#include <memory>
#include <vector>

class VulkanQueue;
class VulkanFence;
class VulkanDevice;

class VulkanSwapChain
{
public:
	enum class Status
	{
		Healthy = 0,
		OutOfDate = -1,
		SurfaceLost = -2,
	};

	VulkanSwapChain(VkInstance instance, VulkanDevice* device, void* windowHandle, PixelFormat& outPixelFormat, uint32 width, uint32 height,
		uint32* outDesiredNumBackBuffers, std::vector<VkImage>& outImages, int8 lockToVsync);

	void Destroy();

	Status Present(VulkanQueue* GfxQueue, VulkanQueue* PresentQueue, VkSemaphore* BackBufferRenderingDoneSemaphore);

	inline int8 DoesLockToVsync() 
	{ 
		return m_LockToVsync;
	}

protected:
	int32 AcquireImageIndex(VkSemaphore* OutSemaphore);
	
protected:
	VkSwapchainKHR m_SwapChain;
	VkSurfaceKHR m_Surface;
	VulkanDevice* m_Device;

	int32 m_CurrentImageIndex;
	int32 m_SemaphoreIndex;
	uint32 m_NumPresentCalls;
	uint32 m_NumAcquireCalls;
	VkInstance m_Instance;
	std::vector<VkSemaphore> m_ImageAcquiredSemaphore;
	std::vector<VulkanFence*> m_ImageAcquiredFences;
	int8 m_LockToVsync;
	uint32 m_PresentID = 0;

	friend class VulkanViewport;
	friend class VulkanQueue;
};