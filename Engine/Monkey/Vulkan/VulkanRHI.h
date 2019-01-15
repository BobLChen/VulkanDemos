#pragma once

#include "Common/Common.h"
#include "VulkanPlatform.h"
#include "VulkanGlobals.h"
#include <string>

class VulkanDevice;
class VulkanQueue;

class VulkanRHI
{
public:
	VulkanRHI();

	virtual ~VulkanRHI();

	virtual void Init();

	virtual void PostInit();

	virtual void Shutdown();;

	virtual void InitInstance();

	static void SavePipelineCache();

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
    
	inline bool SupportsDebugUtilsExt() const
	{
		return m_SupportsDebugUtilsExt;
	}
	
	virtual const char* GetName() 
	{ 
		return "Vulkan";
	}

	static void RecreateSwapChain(void* newNativeWindow);

protected:
	void PooledUniformBuffersBeginFrame();

	void ReleasePooledUniformBuffers();

	void CreateInstance();

	void SelectAndInitDevice();

	void InitGPU(VkDevice device);

	void InitDevice(VkDevice device);

	static void GetInstanceLayersAndExtensions(std::vector<const char*>& outInstanceExtensions, std::vector<const char*>& outInstanceLayers, bool& outDebugUtils);

protected:
    
	VkInstance m_Instance;
	std::shared_ptr<VulkanDevice> m_Device;
	std::vector<const char*> m_InstanceLayers;
	std::vector<const char*> m_InstanceExtensions;
	std::vector<std::shared_ptr<VulkanDevice>> m_Devices;
	
	bool m_SupportsDebugUtilsExt;
};
