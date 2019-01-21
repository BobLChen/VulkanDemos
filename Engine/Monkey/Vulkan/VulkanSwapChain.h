#include "VulkanPlatform.h"
#include "VulkanDevice.h"
#include "Common/Common.h"

#include <memory>
#include <vector>

class VulkanQueue;

class VulkanSwapChain
{
public:
	enum class Status
	{
		Healthy = 0,
		OutOfDate = -1,
		SurfaceLost = -2,
	};

	VulkanSwapChain(VkInstance instance, std::shared_ptr<VulkanDevice> device, void* windowHandle, PixelFormat& outPixelFormat, uint32 width, uint32 height,
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
	std::shared_ptr<VulkanDevice> m_Device;
	VkSurfaceKHR m_Surface;

	int32 m_CurrentImageIndex;
	int32 m_SemaphoreIndex;
	uint32 m_NumPresentCalls;
	uint32 m_NumAcquireCalls;
	VkInstance m_Instance;
	std::vector<VkSemaphore> m_ImageAcquiredSemaphore;
	std::vector<VkFence> m_ImageAcquiredFences;
	int8 m_LockToVsync;
	uint32 m_PresentID = 0;

	friend class VulkanViewport;
	friend class VulkanQueue;
};