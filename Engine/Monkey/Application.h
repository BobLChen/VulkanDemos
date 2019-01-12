#pragma once

#include "Common/Common.h"

#include <vector>
#include <string>
#include <vulkan/vulkan.h>

class Sample;

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallBack(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);

class Application
{
public:
    
    explicit Application();
    Application(int width, int height, const std::string& title);
	
    virtual ~Application();
    
    void AddValidationLayer(const std::string& layer);
    void AddInstanceExtension(const std::string& name);
    void AddPhysicalDeviceExtension(const std::string& name);
    
    void Run(Sample* sample);
    
	const VkInstance& GetVkInstance() const
	{
		return m_VkInstance;
	}
	
    friend VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallBack(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg, void* userData);
    
protected:
    
    virtual void OnDestory() = 0;
    virtual bool InitVulkanSurface() = 0;
    virtual bool InitWindow() = 0;
    virtual void OnLoop() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnKeyDown(int key) = 0;
	virtual void OnKeyUP(int key) = 0;
	virtual void OnMouseMove(int x, int y) = 0;
	virtual void OnMouseDown(int x, int y, int type) = 0;
	virtual void OnMouseUP(int x, int y, int type) = 0;
    
	bool Init();
    bool CreateVKInstance();
    void DestoryVKResource();
    bool CheckValidationLayerSupport();
    bool CheckInstanceExtensionsSupport();
    bool SetupVulkanDebugCallBack();
    void DestoryDebugCallBack();
    bool CreatePhysicalDevice();
    bool CreateLogicalDevice();
	bool CreateSwapChain();
    bool ValidatePhysicalDevice(const VkPhysicalDevice& vkPhysicalDevice);
    bool CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice& vkPhysicalDevice);
    void OnDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg);

public:
	int m_Width;
	int m_Height;
	std::string m_Title;
	Sample* m_Sample;
    
    std::vector<std::string> m_ValidationLayers;
    std::vector<std::string> m_InstanceExtensions;
    std::vector<std::string> m_PhysicalDeviceExtensions;

	uint32_t m_GraphicsFamilyIndex;
	uint32_t m_ComputeFamilyIndex;
	uint32_t m_TransferFamilyIndex;
	uint32_t m_PresentFamilyIndex;
	VkDebugReportCallbackEXT m_VKDebugCallback;
	VkInstance m_VkInstance;
    VkPhysicalDevice m_VKPhysicalDevice;
    VkDevice m_VKDevice;
    VkQueue m_GraphicsQueue;
    VkSurfaceKHR m_VKSurface;
	VkSwapchainKHR m_VKSwapChain;
	VkFormat m_VKSwapChainFormat;
	VkExtent2D m_VKSwapChainExtent;
	std::vector<VkImage> m_VKSwapChainImages;
    std::vector<VkImageView> m_VKSwapChainImageViews;
};


