#include "Application.h"
#include "Sample.h"

#include <set>
#include <vector>

NS_MONKEY_BEGIN

#define VK_DESTORY_DEBUG_REPORT_CALLBACK_EXT_NAME "vkDestroyDebugReportCallbackEXT"
#define VK_CREATE_DEBUG_REPORT_CALLBACK_EXT_NAME  "vkCreateDebugReportCallbackEXT"

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallBack(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData)
{
    Application* app = reinterpret_cast<Application*>(userData);
    app->OnDebugCallback(flags, objType, obj, location, code, layerPrefix, msg);
    return VK_FALSE;
}

Application::Application()
    : m_Width(800)
    , m_Height(600)
    , m_Title("Application")
    , m_Sample(nullptr)
	, m_VKDebugCallback(nullptr)
	, m_VkInstance(VK_NULL_HANDLE)
	, m_VKPhysicalDevice(VK_NULL_HANDLE)
	, m_VKDevice(VK_NULL_HANDLE)
	, m_GraphicsQueue(VK_NULL_HANDLE)
	, m_VKSurface(VK_NULL_HANDLE)
	, m_VKSwapChain(VK_NULL_HANDLE)
	, m_GraphicsFamilyIndex(0)
	, m_ComputeFamilyIndex(0)
	, m_TransferFamilyIndex(0)
	, m_PresentFamilyIndex(0)
{

}

Application::Application(int width, int height, const std::string& title)
    : m_Width(width)
    , m_Height(height)
    , m_Title(title)
    , m_Sample(nullptr)
	, m_VKDebugCallback(nullptr)
	, m_VkInstance(VK_NULL_HANDLE)
	, m_VKPhysicalDevice(VK_NULL_HANDLE)
	, m_VKDevice(VK_NULL_HANDLE)
	, m_GraphicsQueue(VK_NULL_HANDLE)
	, m_VKSurface(VK_NULL_HANDLE)
	, m_VKSwapChain(VK_NULL_HANDLE)
	, m_GraphicsFamilyIndex(0)
	, m_ComputeFamilyIndex(0)
	, m_TransferFamilyIndex(0)
	, m_PresentFamilyIndex(0)
{
    
}

Application::~Application()
{
    
}

bool Application::CheckValidationLayerSupport()
{
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    bool layerFound = true;
    for (int i = 0; i < m_ValidationLayers.size(); ++i) {
        bool found = false;
        for (int j = 0; j < availableLayers.size(); ++j) {
            if (m_ValidationLayers[i] == availableLayers[j].layerName) {
                found = true;
                break;
            }
        }
        if (!found) {
            printf("Layer not support: %s\n", m_ValidationLayers[i].c_str());
        }
        layerFound &= found;
    }
    
    return layerFound;
}

bool Application::CheckInstanceExtensionsSupport()
{
    uint32_t vkExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, nullptr);
    std::vector<VkExtensionProperties> vkExtensionNames(vkExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &vkExtensionCount, vkExtensionNames.data());
    
    bool extensionFound = true;
    for (int i = 0; i < m_InstanceExtensions.size(); ++i) {
        bool found = false;
        for (int j = 0; j < vkExtensionNames.size(); ++j) {
            if (m_InstanceExtensions[i] == vkExtensionNames[j].extensionName) {
                found = true;
                continue;
            }
        }
        if (!found) {
            printf("GLFW extension not found:%s\n", m_InstanceExtensions[i].c_str());
        }
        extensionFound &= found;
    }
    
    return extensionFound;
}

void Application::OnDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg)
{
    m_Sample->OnDebugCallback(flags, objType, obj, location, code, layerPrefix, msg);
}

bool Application::SetupVulkanDebugCallBack()
{
    VkDebugReportCallbackCreateInfoEXT debugInfo = {};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    debugInfo.pfnCallback = VulkanDebugCallBack;
    debugInfo.pUserData = this;
    
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_VkInstance, VK_CREATE_DEBUG_REPORT_CALLBACK_EXT_NAME);
    if (func != nullptr) {
		return func(m_VkInstance, &debugInfo, nullptr, &m_VKDebugCallback) == VK_SUCCESS;
    }
	else {
		return false;
	}
}

bool Application::CreateVKInstance()
{
    if (!CheckInstanceExtensionsSupport()) {
        printf("Failed init vulkan, extension not support.\n");
        return false;
    }
    
    if (!CheckValidationLayerSupport()) {
        printf("Failed init vulkan, validation layer not support.\n");
        return false;
    }
    
    VkApplicationInfo vkAppInfo = {};
    vkAppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    vkAppInfo.pNext = nullptr;
    vkAppInfo.pApplicationName = m_Title.c_str();
    vkAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    vkAppInfo.apiVersion = VK_API_VERSION_1_1;
    vkAppInfo.pEngineName = "Monkey";
    vkAppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    
    std::vector<const char*> validationLayers(m_ValidationLayers.size());
    for (int i = 0; i < m_ValidationLayers.size(); ++i) {
        validationLayers[i] = m_ValidationLayers[i].c_str();
    }
    std::vector<const char*> extensionNames(m_InstanceExtensions.size());
    for (int i = 0; i < m_InstanceExtensions.size(); ++i) {
        extensionNames[i] = m_InstanceExtensions[i].c_str();
    }
    
    VkInstanceCreateInfo vkCreateInfo = {};
    vkCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkCreateInfo.pApplicationInfo = &vkAppInfo;
    vkCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
    vkCreateInfo.ppEnabledLayerNames = validationLayers.data();
    vkCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_InstanceExtensions.size());
    vkCreateInfo.ppEnabledExtensionNames = extensionNames.data();
    
    if (vkCreateInstance(&vkCreateInfo, nullptr, &m_VkInstance) != VK_SUCCESS) {
        return false;
    }
    
    printf("Vulkan init success.\nLayerCount:%d ExtensionCount:%d\n", vkCreateInfo.enabledLayerCount, vkCreateInfo.enabledExtensionCount);
    
    if (validationLayers.size() > 0 && !SetupVulkanDebugCallBack()) {
        printf("Failed setup debug callback.\n");
        return false;
    }
    
    return true;
}

bool Application::Init()
{
	if (!InitWindow()) {
		printf("Failed init window.\n");
		return false;
	}

	if (!CreateVKInstance()) {
		printf("Failed init vulkan instance.\n");
		return false;
	}

	if (!InitVulkanSurface()) {
		printf("Failed init vulkan surface.\n");
		return false;
	}

	if (!CreatePhysicalDevice()) {
		printf("Failed create physical device.\n");
		return false;
	}

	if (!CreateLogicalDevice()) {
		printf("Failed create graphics logical device.\n");
		return false;
	}

	if (!CreateSwapChain()) {
		printf("Failed create swap chain.\n");
		return false;
	}

	return true;
}

void Application::Run(Sample *sample)
{
    if (sample == nullptr) {
        printf("Failed run application, found emptry Sample.\n");
        return;
    }
    
    m_Sample = sample;
    m_Sample->SetupApplication(this);
    
	if (!Init()) {
		printf("Failed init.\n");
		return;
	}
    
	m_Sample->OnInit();
    OnLoop();
	m_Sample->OnDestory();
    OnDestory();
	DestoryVKResource();
}

bool Application::CreateSwapChain()
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_VKPhysicalDevice, m_VKSurface, &capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_VKPhysicalDevice, m_VKSurface, &formatCount, nullptr);
	if (formatCount == 0) {
		return false;
	}
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_VKPhysicalDevice, m_VKSurface, &formatCount, formats.data());

	VkSurfaceFormatKHR surfaceFormat;
	surfaceFormat.format = VK_FORMAT_UNDEFINED;
	for (int i = 0; i < formats.size(); ++i) {
		if (formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			surfaceFormat = formats[i];
			break;
		}
	}
	if (surfaceFormat.format == VK_FORMAT_UNDEFINED) {
		surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM;
		surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	}
	
	uint32_t presentCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_VKPhysicalDevice, m_VKSurface, &presentCount, nullptr);
	if (presentCount == 0) {
		return false;
	}
	std::vector<VkPresentModeKHR> presentModes(presentCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_VKPhysicalDevice, m_VKSurface, &presentCount, presentModes.data());

	VkPresentModeKHR presentMode;
	bool found = false;
	for (int i = 0; i < presentModes.size(); ++i) {
		if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			found = true;
			presentMode = presentModes[i];
			break;
		}
	}
	if (!found) {
		presentMode = presentModes[0];
	}

	uint32_t imageCount = 3;
	if (capabilities.maxImageCount > 0) {
		imageCount = capabilities.maxImageCount < 3 ? capabilities.maxImageCount : 3;
	}

	std::vector<uint32_t> queueFamilyIndices = {};

	VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
	swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainCreateInfo.surface = m_VKSurface;
	swapChainCreateInfo.minImageCount = imageCount;
	swapChainCreateInfo.imageFormat = surfaceFormat.format;
	swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapChainCreateInfo.imageExtent = capabilities.currentExtent;
	swapChainCreateInfo.imageArrayLayers = 1;
	swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainCreateInfo.preTransform = capabilities.currentTransform;
	swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainCreateInfo.presentMode = presentMode;
	swapChainCreateInfo.clipped = VK_TRUE;
	swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(m_VKDevice, &swapChainCreateInfo, nullptr, &m_VKSwapChain) != VK_SUCCESS) {
		printf("Failed create swap chain.\n");
		return false;
	}

	vkGetSwapchainImagesKHR(m_VKDevice, m_VKSwapChain, &imageCount, nullptr);
	m_VKSwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_VKDevice, m_VKSwapChain, &imageCount, m_VKSwapChainImages.data());

	m_VKSwapChainFormat = surfaceFormat.format;
	m_VKSwapChainExtent = capabilities.currentExtent;
    
    m_VKSwapChainImageViews.resize(m_VKSwapChainImages.size());
    for (int i = 0; i < m_VKSwapChainImageViews.size(); ++i) {
        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_VKSwapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_VKSwapChainFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(m_VKDevice, &createInfo, nullptr, &m_VKSwapChainImageViews[i]) != VK_SUCCESS) {
            printf("Failed to create swap chain image view.\n");
            return false;
        }
    }
    
	return true;
}

void Application::DestoryVKResource()
{
	if (m_ValidationLayers.size() > 0) {
		DestoryDebugCallBack();
	}
    for (int i = 0; i < m_VKSwapChainImageViews.size(); ++i) {
        vkDestroyImageView(m_VKDevice, m_VKSwapChainImageViews[i], nullptr);
    }
	vkDestroySwapchainKHR(m_VKDevice, m_VKSwapChain, nullptr);
	vkDestroySurfaceKHR(m_VkInstance, m_VKSurface, nullptr);
    vkDestroyDevice(m_VKDevice, nullptr);
    vkDestroyInstance(m_VkInstance, nullptr);
}

void Application::AddInstanceExtension(const std::string &name)
{
    for (int i = 0; i < m_InstanceExtensions.size(); ++i) {
        if (m_InstanceExtensions[i] == name) {
            return;
        }
    }
    m_InstanceExtensions.push_back(name);
}

void Application::AddPhysicalDeviceExtension(const std::string &name)
{
    for (int i = 0; i < m_PhysicalDeviceExtensions.size(); ++i) {
        if (m_PhysicalDeviceExtensions[i] == name) {
            return;
        }
    }
    m_PhysicalDeviceExtensions.push_back(name);
}

void Application::AddValidationLayer(const std::string &layer)
{
    for (int i = 0; i < m_ValidationLayers.size(); ++i) {
        if (m_ValidationLayers[i] == layer) {
            return;
        }
    }
    m_ValidationLayers.push_back(layer);
}

bool Application::CreateLogicalDevice()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_VKPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_VKPhysicalDevice, &queueFamilyCount, queueFamilies.data());
    
	int graphicsQueueCount  = 0;
	int computeQueueCount   = 0;
	int transferQueueCount  = 0;
	bool foundPresent = false;
    for (int i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueCount <= 0) {
            continue;
        }
        if (graphicsQueueCount == 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsQueueCount = queueFamilies[i].queueCount;
			m_GraphicsFamilyIndex = i;
        }
		if (computeQueueCount == 0 && queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			computeQueueCount = queueFamilies[i].queueCount;
			m_ComputeFamilyIndex = i;
		}
		if (transferQueueCount ==0 && queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			transferQueueCount = queueFamilies[i].queueCount;
			m_TransferFamilyIndex = i;
		}
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_VKPhysicalDevice, i, m_VKSurface, &presentSupport);
        if (presentSupport && !foundPresent) {
			foundPresent = true;
			m_PresentFamilyIndex = i;
        }
    }
    
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> familyIndexArray = {m_GraphicsFamilyIndex, m_PresentFamilyIndex, m_ComputeFamilyIndex, m_TransferFamilyIndex};
	float queuePriorities = 1.0f;
	for (int familyIndex : familyIndexArray) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = familyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriorities;
		queueCreateInfos.push_back(queueCreateInfo);
	}

    std::vector<const char*> layerNames(m_ValidationLayers.size());
    for (int i = 0; i < m_ValidationLayers.size(); ++i) {
        layerNames[i] = m_ValidationLayers[i].c_str();
    }
	std::vector<const char*> deviceExtensions(m_PhysicalDeviceExtensions.size());
	for (int i = 0; i < m_PhysicalDeviceExtensions.size(); ++i) {
		deviceExtensions[i] = m_PhysicalDeviceExtensions[i].c_str();
	}

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.enabledLayerCount = m_ValidationLayers.size();
    createInfo.ppEnabledLayerNames = layerNames.data();
    
    if (vkCreateDevice(m_VKPhysicalDevice, &createInfo, nullptr, &m_VKDevice) != VK_SUCCESS) {
        printf("Failed create logic device.\n");
        return false;
    }
    
    vkGetDeviceQueue(m_VKDevice, m_GraphicsFamilyIndex, 0, &m_GraphicsQueue);
    return true;
}

bool Application::ValidatePhysicalDevice(const VkPhysicalDevice& vkPhysicalDevice)
{
    if (!CheckPhysicalDeviceExtensionSupport(vkPhysicalDevice)) {
        return false;
    }
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilies.data());
    
    bool foundGraphics = false;
    bool foundCompute  = false;
    bool foundTransfer = false;
    VkBool32 presetSupport = false;
    for (int i = 0; i < queueFamilies.size(); ++i) {
        if (queueFamilies[i].queueCount <= 0) {
            continue;
        }
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            foundGraphics = true;
        }
        if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            foundCompute  = true;
        }
        if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT) {
            foundTransfer = true;
        }
        VkBool32 support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, i, m_VKSurface, &support);
        if (support) {
            presetSupport = true;
        }
    }
    
    return foundGraphics && foundCompute && foundTransfer && presetSupport;
}

bool Application::CheckPhysicalDeviceExtensionSupport(const VkPhysicalDevice &vkPhysicalDevice)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(vkPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());
    
    bool valid = true;
    for (int i = 0; i < m_PhysicalDeviceExtensions.size(); ++i) {
        bool found = false;
        for (int j = 0; j < availableExtensions.size(); ++j) {
            if (m_PhysicalDeviceExtensions[i] == availableExtensions[j].extensionName) {
                found = true;
            }
        }
        if (!found) {
            valid = false;
            printf("Device extension not support : %s\n", m_PhysicalDeviceExtensions[i].c_str());
        }
    }
    
    return valid;
}

bool Application::CreatePhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        return false;
    }
    
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, devices.data());
    
    int maxScore = 0;
    int maxIndex = 0;
    for (int i = 0; i < devices.size(); ++i) {
        if (!ValidatePhysicalDevice(devices[i])) {
            continue;
        }
        
        int score = 0;
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(devices[i], &deviceProperties);

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 10;
            VkPhysicalDeviceMemoryProperties memoryProperties;
            vkGetPhysicalDeviceMemoryProperties(devices[i], &memoryProperties);
            for (int j = 0; j < memoryProperties.memoryHeapCount; ++j) {
                if (memoryProperties.memoryHeaps[j].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                    score += (memoryProperties.memoryHeaps[j].size / 1024 / 1024 / 1024);
                }
            }
        }
        
        if (maxScore > score) {
            maxScore = score;
            maxIndex = i;
        }
    }
    
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(devices[maxIndex], &deviceProperties);
    m_VKPhysicalDevice = devices[maxIndex];
    printf("Select most powerful physical device:%s\n", deviceProperties.deviceName);
    return true;
}

void Application::DestoryDebugCallBack()
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_VkInstance, VK_DESTORY_DEBUG_REPORT_CALLBACK_EXT_NAME);
    if (func != nullptr) {
        func(m_VkInstance, m_VKDebugCallback, nullptr);
    }
}

NS_MONKEY_END
