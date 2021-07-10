# 绘制三角形

使用Vulkan绘制出一个三角形实属不易，由于Vulkan较底层，需要应用程序处理的事情较多。整个代码量估计可能在1200行左右，对于OpenGL、Directx11、WebGL等来讲，这个代码量还是比较庞大。本文只是简单阐述了整个流程，具体某个参数的含义未详细解释。建议可以阅读https://www.cnblogs.com/heitao/tag/Vulkan/或者https://vulkan-tutorial.com/文章。

## 潜规则

Vulkan的API里面有一些潜规则。

- 大多数对象的创建，都需要与之相应的描述结构体(与DX12类似)。例如:创建`VkInstance`需要`VkInstanceCreateInfo`来描述，而不再是通过众多参数完成。
- Vulkan对象都是以`Vk`开头。例如:`VkInstance`。
- Vulkan API则以`v`k开头。例如:`vkCreateInstance`。

## Vulkan Instance

代码位于：`Engine\Monkey\Vulkan\VulkanRHI.cpp`

### 校验层以及扩展

在创建Vulkan的Instance时，我们需要打开一些扩展以及校验层。扩展表示一些额外功能，例如Nvidia新出的RTX系列显卡的光追技术，在其它系列或者厂商的显卡上是得不到支持的，因此需要特定的开启扩展。校验层则用来校验我们是否正确使用Vulkan API，可以简单的理解为用于Debug的功能。

通过`vkEnumerateInstanceExtensionProperties`我们可以获取到`Instance`所有的扩展，如果传入一个确切的"校验层"名称，则可以获取该校验层的所有扩展。这个其实也不难理解，因为校验层也是以扩展的形式插入进来的。

通过`vkEnumerateDeviceExtensionProperties`我们则可以获取`Device`的所有扩展。以下代码是获取校验层以及扩展的代码。可以在`Engine\Monkey\Vulkan\VulkanLayers.cpp`中找到。可以从代码中看到，针对不同的平台设置了不同的扩展以及校验层，并且内置的这些不一定会被开启。

```c++
struct VulkanLayerExtension
{
	VulkanLayerExtension();
	void AddUniqueExtensionNames(std::vector<std::string>& outExtensions);
	void AddUniqueExtensionNames(std::vector<const char*>& outExtensions);

	VkLayerProperties layerProps;
	std::vector<VkExtensionProperties> extensionProps;
};

static const char* G_ValidationLayersInstance[] =
{
#if PLATFORM_WINDOWS
	"VK_LAYER_LUNARG_standard_validation",
#elif PLATFORM_MAC
	"VK_LAYER_LUNARG_standard_validation",
    "VK_LAYER_GOOGLE_unique_objects",
    "VK_LAYER_GOOGLE_threading",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_parameter_validation",
    "VK_LAYER_LUNARG_object_tracker",
#elif PLATFORM_IOS
    "MoltenVK",
#elif PLATFORM_ANDROID
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
#elif PLATFORM_LINUX
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
#elif PLATFORM_ANDROID
	
#endif
	nullptr
};

static const char* G_ValidationLayersDevice[] =
{
#if PLATFORM_WINDOWS
	"VK_LAYER_LUNARG_standard_validation",
#elif PLATFORM_IOS
    "MoltenVK",
#elif PLATFORM_MAC
    "VK_LAYER_LUNARG_standard_validation",
    "VK_LAYER_GOOGLE_unique_objects",
    "VK_LAYER_GOOGLE_threading",
    "VK_LAYER_LUNARG_core_validation",
    "VK_LAYER_LUNARG_parameter_validation",
    "VK_LAYER_LUNARG_object_tracker",
#elif PLATFORM_ANDROID
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
#elif PLATFORM_LINUX
	"VK_LAYER_GOOGLE_threading",
	"VK_LAYER_LUNARG_parameter_validation",
	"VK_LAYER_LUNARG_object_tracker",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_GOOGLE_unique_objects",
	"VK_LAYER_LUNARG_core_validation",
#elif PLATFORM_ANDROID

#endif
	nullptr
};

static const char* G_InstanceExtensions[] =
{
#if PLATFORM_WINDOWS
	
#elif PLATFORM_MAC

#elif PLATFORM_IOS
    
#elif PLATFORM_LINUX

#elif PLATFORM_ANDROID
    
#endif
	nullptr
};

static const char* G_DeviceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,

#if PLATFORM_WINDOWS

#elif PLATFORM_MAC

#elif PLATFORM_IOS

#elif PLATFORM_LINUX

#elif PLATFORM_ANDROID
	
#endif

	nullptr
};

static FORCE_INLINE void EnumerateInstanceExtensionProperties(const char* layerName, VulkanLayerExtension& outLayer)
{
	uint32 count = 0;
	vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateInstanceExtensionProperties(layerName, &count, outLayer.extensionProps.data());
}

static FORCE_INLINE void EnumerateDeviceExtensionProperties(VkPhysicalDevice device, const char* layerName, VulkanLayerExtension& outLayer)
{
	uint32 count = 0;
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, outLayer.extensionProps.data());
}

static FORCE_INLINE int32 FindLayerIndexInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName)
{
	for (int32 i = 0; i < layers.size(); ++i) 
	{
		if (strcmp(layers[i].layerProps.layerName, layerName) == 0) 
		{
			return i;
		}
	}
	return -1;
}

static FORCE_INLINE bool FindLayerInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName)
{
	return FindLayerIndexInList(layers, layerName) != -1;
}

static FORCE_INLINE bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName, const char*& foundLayer)
{
	for (int32 i = 0; i < layers.size(); ++i) 
	{
		for (int32 j = 0; j < layers[i].extensionProps.size(); ++j)
		{
			if (strcmp(layers[i].extensionProps[j].extensionName, extensionName) == 0)
			{
				foundLayer = layers[i].layerProps.layerName;
				return true;
			}
		}
	}
	return false;
}

static FORCE_INLINE bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName)
{
	const char* dummy = nullptr;
	return FindLayerExtensionInList(layers, extensionName, dummy);
}

static FORCE_INLINE void TrimDuplicates(std::vector<const char*>& arr)
{
	for (int32 i = (int32)arr.size() - 1; i >= 0; --i)
	{
		bool found = false;
		for (int32 j = i - 1; j >= 0; --j)
		{
			if (strcmp(arr[i], arr[j]) == 0) {
				found = true;
				break;
			}
		}
		if (found) {
			arr.erase(arr.begin() + i);
		}
	}
}

VulkanLayerExtension::VulkanLayerExtension()
{
	memset(&layerProps, 0, sizeof(VkLayerProperties));
}

void VulkanLayerExtension::AddUniqueExtensionNames(std::vector<std::string>& outExtensions)
{
	for (int32 i = 0; i < extensionProps.size(); ++i) 
	{
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
	}
}

void VulkanLayerExtension::AddUniqueExtensionNames(std::vector<const char*>& outExtensions)
{
	for (int32 i = 0; i < extensionProps.size(); ++i) 
	{
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
	}
}

void VulkanRHI::GetInstanceLayersAndExtensions(std::vector<const char*>& outInstanceExtensions, std::vector<const char*>& outInstanceLayers, bool& outDebugUtils)
{
	outDebugUtils = false;

	std::vector<VulkanLayerExtension> globalLayerExtensions(1);
	EnumerateInstanceExtensionProperties(nullptr, globalLayerExtensions[0]);

	std::vector<std::string> foundUniqueExtensions;
	for (int32 i = 0; i < globalLayerExtensions[0].extensionProps.size(); ++i) 
	{
		StringUtils::AddUnique(foundUniqueExtensions, globalLayerExtensions[0].extensionProps[i].extensionName);
	}
	
	uint32 instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	std::vector<VkLayerProperties> globalLayerProperties(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, globalLayerProperties.data());

	std::vector<std::string> foundUniqueLayers;
	for (int32 i = 0; i < globalLayerProperties.size(); ++i) 
	{
		VulkanLayerExtension layer;
		layer.layerProps = globalLayerProperties[i];
		EnumerateInstanceExtensionProperties(globalLayerProperties[i].layerName, layer);
		layer.AddUniqueExtensionNames(foundUniqueExtensions);
		StringUtils::AddUnique(foundUniqueLayers, globalLayerProperties[i].layerName);
		globalLayerExtensions.push_back(layer);
	}

	for (const std::string& name : foundUniqueLayers) 
	{
		MLOG("- Found instance layer %s", name.c_str());
	}

	for (const std::string& name : foundUniqueExtensions) 
	{
		MLOG("- Found instance extension %s", name.c_str());
	}
	
#if MONKEY_DEBUG
	for (int32 i = 0; G_ValidationLayersInstance[i] != nullptr; ++i) 
	{
		const char* currValidationLayer = G_ValidationLayersInstance[i];
		bool found = FindLayerInList(globalLayerExtensions, currValidationLayer);
		if (found) 
		{
			outInstanceLayers.push_back(currValidationLayer);
		}
		else 
		{
			MLOG("Unable to find Vulkan instance validation layer '%s'", currValidationLayer);
		}
	}

    const char* foundDebugUtilsLayer = nullptr;
    outDebugUtils = FindLayerExtensionInList(globalLayerExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME, foundDebugUtilsLayer);
    if (outDebugUtils && *foundDebugUtilsLayer)
    {
        outInstanceLayers.push_back(foundDebugUtilsLayer);
    }

    if (outDebugUtils && FindLayerExtensionInList(globalLayerExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME))
    {
        outInstanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
#endif // MONKEY_DEBUG

	if (outDebugUtils && FindLayerExtensionInList(globalLayerExtensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		outInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
	
	std::vector<const char*> platformExtensions;
	VulkanPlatform::GetInstanceExtensions(platformExtensions);

	for (const char* extension : platformExtensions) 
	{
		if (FindLayerExtensionInList(globalLayerExtensions, extension))
		{
			outInstanceExtensions.push_back(extension);
		}
	}

	for (int32 i = 0; G_InstanceExtensions[i] != nullptr; ++i)
	{
		if (FindLayerExtensionInList(globalLayerExtensions, G_InstanceExtensions[i]))
		{
			outInstanceExtensions.push_back(G_InstanceExtensions[i]);
		}
	}
    
	TrimDuplicates(outInstanceLayers);
	if (outInstanceLayers.size() > 0)
	{
		MLOG("Using instance layers");
		for (const char* layer : outInstanceLayers)
		{
			MLOG("* %s", layer);
		}
	}
	else
	{
		MLOG("Not using instance layers");
	}

	TrimDuplicates(outInstanceExtensions);
	if (outInstanceExtensions.size() > 0)
	{
		MLOG("Using instance extensions");
		for (const char* extension : outInstanceExtensions)
		{
			MLOG("* %s", extension);
		}
	}
	else
	{
		MLOG("Not using instance extensions");
	}
}

void VulkanDevice::GetDeviceExtensionsAndLayers(std::vector<const char*>& outDeviceExtensions, std::vector<const char*>& outDeviceLayers, bool& bOutDebugMarkers)
{
    bOutDebugMarkers = false;
    
    uint32 count = 0;
    vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &count, nullptr);
    std::vector<VkLayerProperties> properties(count);
    vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &count, properties.data());
    
    std::vector<VulkanLayerExtension> deviceLayerExtensions(count + 1);
    for (int32 index = 1; index < deviceLayerExtensions.size(); ++index) {
        deviceLayerExtensions[index].layerProps = properties[index - 1];
    }
    
    std::vector<std::string> foundUniqueLayers;
    std::vector<std::string> foundUniqueExtensions;
    for (int32 index = 0; index < deviceLayerExtensions.size(); ++index)
    {
        if (index == 0)
        {
            EnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, deviceLayerExtensions[index]);
        }
        else
        {
            StringUtils::AddUnique(foundUniqueLayers, deviceLayerExtensions[index].layerProps.layerName);
            EnumerateDeviceExtensionProperties(m_PhysicalDevice, deviceLayerExtensions[index].layerProps.layerName, deviceLayerExtensions[index]);
        }
        
        deviceLayerExtensions[index].AddUniqueExtensionNames(foundUniqueExtensions);
    }
    
    for (const std::string& name : foundUniqueLayers)
    {
        MLOG("- Found device layer %s", name.c_str());
    }
    
    for (const std::string& name : foundUniqueExtensions)
    {
        MLOG("- Found device extension %s", name.c_str());
    }

#if MONKEY_DEBUG
    for (uint32 layerIndex = 0; G_ValidationLayersDevice[layerIndex] != nullptr; ++layerIndex)
    {
        bool bValidationFound = false;
        const char* currValidationLayer = G_ValidationLayersDevice[layerIndex];
        for (int32 index = 1; index < deviceLayerExtensions.size(); ++index)
        {
            if (strcmp(deviceLayerExtensions[index].layerProps.layerName, currValidationLayer) == 0)
            {
                bValidationFound = true;
                outDeviceLayers.push_back(currValidationLayer);
                break;
            }
        }
        
        if (!bValidationFound)
        {
            MLOG("Unable to find Vulkan device validation layer '%s'", currValidationLayer);
        }
    }
#endif
    
    std::vector<const char*> availableExtensions;
    for (int32 extIndex = 0; extIndex < deviceLayerExtensions[0].extensionProps.size(); ++extIndex)
    {
        availableExtensions.push_back(deviceLayerExtensions[0].extensionProps[extIndex].extensionName);
    }
    
    for (int32 layerIndex = 0; layerIndex < outDeviceLayers.size(); ++layerIndex)
    {
        int32 findLayerIndex;
        for (findLayerIndex = 1; findLayerIndex < deviceLayerExtensions.size(); ++findLayerIndex)
        {
            if (strcmp(deviceLayerExtensions[findLayerIndex].layerProps.layerName, outDeviceLayers[layerIndex]) == 0)
            {
                break;
            }
        }
        
        if (findLayerIndex < deviceLayerExtensions.size())
        {
            deviceLayerExtensions[findLayerIndex].AddUniqueExtensionNames(availableExtensions);
        }
    }
    
    TrimDuplicates(availableExtensions);
    
    auto ListContains = [](const std::vector<const char*>& arr, const char* name) -> bool
    {
        for (const char* element : arr)
        {
            if (strcmp(element, name) == 0)
            {
                return true;
            }
        }
        return false;
    };
    
    std::vector<const char*> platformExtensions;
    VulkanPlatform::GetDeviceExtensions(platformExtensions);
    for (const char* platformExtension : platformExtensions)
    {
        if (ListContains(availableExtensions, platformExtension))
        {
            outDeviceExtensions.push_back(platformExtension);
            break;
        }
    }
    
    for (uint32 index = 0; G_DeviceExtensions[index] != nullptr; ++index)
    {
        if (ListContains(availableExtensions, G_DeviceExtensions[index]))
        {
            outDeviceExtensions.push_back(G_DeviceExtensions[index]);
        }
    }
    
    if (outDeviceExtensions.size() > 0)
    {
        MLOG("Using device extensions");
        for (const char* extension : outDeviceExtensions)
        {
            MLOG("* %s", extension);
        }
    }
    
    if (outDeviceLayers.size() > 0)
    {
        MLOG("Using device layers");
        for (const char* layer : outDeviceLayers)
        {
            MLOG("* %s", layer);
        }
    }
}

```

### 应用程序信息

创建Instance需要一些额外信息，例如Vulkan版本号、应用程序信息、引擎信息等，因此先准备这些必要的信息。

```c++
template<class T>
static FORCE_INLINE void ZeroVulkanStruct(T& vkStruct, VkStructureType vkType)
{
	vkStruct.sType = vkType;
	memset(((uint8*)&vkStruct) + sizeof(VkStructureType), 0, sizeof(T) - sizeof(VkStructureType));
}
```

```c++
	VkApplicationInfo appInfo;
	ZeroVulkanStruct(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
	appInfo.pApplicationName   = Application::Get().GetPlatformApplication()->GetWindow()->GetTitle();
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName        = ENGINE_NAME;
	appInfo.engineVersion      = VK_MAKE_VERSION(0, 0, 0);
#if PLATFORM_IOS
    appInfo.apiVersion         = VK_API_VERSION_1_0;
#else
    appInfo.apiVersion         = VK_API_VERSION_1_1;
#endif
```

### InstanceCreateInfo

前面提到，创建Vulkan对象很少使用参数的形式，大部分情况都是使用一个结构体用来填充参数，然后进行对象的创建。因此这里举个例子：

```c++
	VkInstanceCreateInfo instanceCreateInfo;
	ZeroVulkanStruct(instanceCreateInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	instanceCreateInfo.pApplicationInfo        = &appInfo;
	instanceCreateInfo.enabledExtensionCount   = uint32_t(m_InstanceExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = m_InstanceExtensions.size() > 0 ? m_InstanceExtensions.data() : nullptr;
	instanceCreateInfo.enabledLayerCount       = uint32_t(m_InstanceLayers.size());
	instanceCreateInfo.ppEnabledLayerNames     = m_InstanceLayers.size() > 0 ? m_InstanceLayers.data() : nullptr;
```

### 创建Instance

参数准备好之后，就可以创建Vulkan Instance了。

```c++
VkResult result = vkCreateInstance(&instanceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Instance);
	if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		MLOG("%s", "Cannot find a compatible Vulkan driver (ICD).");
	}
	else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		std::string missingExtensions;
		uint32 propertyCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data());

		for (const char* extension : m_InstanceExtensions)
		{
			bool found = false;
			for (uint32 i = 0; i < propertyCount; ++i)
			{
				const char* propExtension = properties[i].extensionName;
				if (strcmp(propExtension, extension) == 0) {
					found = true;
					break;
				}
			}
			if (!found) {
				std::string extensionStr(extension);
				missingExtensions += extensionStr + "\n";
			}
		}

		MLOG("Vulkan driver doesn't contain specified extensions:\n%s", missingExtensions.c_str());
	}
	else if (result != VK_SUCCESS)
	{
		MLOG("Vulkan failed to create instance.");
	}
	else
	{
		MLOG("Vulkan successed to create instance.");
	}
```

## 设置Debug回调

之前提到有校验层，可以校验我们的是否使用规范，开启了校验层之后，我们需要知道具体的错误信息。

```c++
void VulkanRHI::SetupDebugLayerCallback()
{
    VkDebugReportCallbackCreateInfoEXT debugInfo;
    ZeroVulkanStruct(debugInfo, VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT);
    debugInfo.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    debugInfo.pfnCallback = VulkanDebugCallBack;
    debugInfo.pUserData   = this;
    
    auto func    = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, VK_CREATE_DEBUG_REPORT_CALLBACK_EXT_NAME);
    bool success = true;
    if (func != nullptr)
    {
        success = func(m_Instance, &debugInfo, nullptr, &m_MsgCallback) == VK_SUCCESS;
    }
    else
    {
        success = false;
    }
    
    if (success)
    {
        MLOG("Setup debug callback success.");
    }
    else
    {
        MLOG("Setup debug callback failed.")
    }
}
```

## 选择合适的物理设备以及创建相应的逻辑设备

GPU物理设备不一定是唯一的，例如2018年的挖矿潮，一台机器上面插了非常之多的GPU。而且非常关键的是显卡厂商还专门推出了矿卡，阉割了图像输出功能。因此我们需要选择正确的显卡以及显卡的某些功能。例如我们可以只计算不输出画面，我们可以用少量的显卡资源用于处理需求。

```c++
void VulkanRHI::SelectAndInitDevice()
{
    uint32 gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);

    if (result == VK_ERROR_INITIALIZATION_FAILED)
    {
        MLOG("%s\n", "Cannot find a compatible Vulkan device or driver. Try updating your video driver to a more recent version and make sure your video card supports Vulkan.");
        Application::Get().OnRequestingExit();
        return;
    }
    
    if (gpuCount == 0)
    {
        MLOG("%s\n", "Couldn't enumerate physical devices! Make sure your drivers are up to date and that you are not pending a reboot.");
        Application::Get().OnRequestingExit();
        return;
    }

	MLOG("Found %d device(s)", gpuCount);

    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    vkEnumeratePhysicalDevices(m_Instance, &gpuCount, physicalDevices.data());
	
	struct DeviceInfo
	{
		std::shared_ptr<VulkanDevice> device;
		uint32 deviceIndex;
	};
	std::vector<DeviceInfo> discreteDevices;
	std::vector<DeviceInfo> integratedDevices;

	for (uint32 index = 0; index < gpuCount; ++index)
	{
		std::shared_ptr<VulkanDevice> newDevice = std::make_shared<VulkanDevice>(physicalDevices[index]);
		bool isDiscrete = newDevice->QueryGPU(index);
        if (isDiscrete) {
            discreteDevices.push_back({newDevice, index});
        }
        else {
            integratedDevices.push_back({newDevice, index});
        }
	}
    
    for (int32 index = 0; index < integratedDevices.size(); ++index)
    {
        discreteDevices.push_back(integratedDevices[index]);
    }
    
    int32 deviceIndex = -1;
    if (discreteDevices.size() > 0)
    {
		int32 preferredVendor = -1;
        if (discreteDevices.size() > 1 && preferredVendor != -1)
        {
            for (int32 index = 0; index < discreteDevices.size(); ++index)
            {
                if (discreteDevices[index].device->GetDeviceProperties().vendorID == preferredVendor)
                {
                    m_Device = discreteDevices[index].device;
                    deviceIndex = discreteDevices[index].deviceIndex;
                    break;
                }
            }
        }
        
        if (deviceIndex == -1)
        {
            m_Device = discreteDevices[0].device;
            deviceIndex = discreteDevices[0].deviceIndex;
        }
    }
    else
    {
        MLOG("%s", "No devices found!");
        deviceIndex = -1;
        Application::Get().OnRequestingExit();
        return;
    }
	
    m_Device->InitGPU(deviceIndex);
}
```

## 交换链

物理设备创建好之后，就可以创建交换链用于存储我们的最终图像。交换链我又将它称之为后台缓冲区，我们需要指定交换链的与神秘平台绑定，例如Window、IOS、MacOS、Android。同时也要指定图像的格式、缓冲区数量、尺寸、呈现方式、混合方法等等。代码可以在`Engine\Monkey\Vulkan\VulkanSwapChain.cpp`找到

```c++
VulkanSwapChain::VulkanSwapChain(VkInstance instance, std::shared_ptr<VulkanDevice> device, PixelFormat& outPixelFormat, uint32 width, uint32 height,
	uint32* outDesiredNumBackBuffers, std::vector<VkImage>& outImages, int8 lockToVsync)
	: m_Instance(instance)
	, m_SwapChain(VK_NULL_HANDLE)
    , m_Surface(VK_NULL_HANDLE)
	, m_Device(device)
	, m_CurrentImageIndex(-1)
	, m_SemaphoreIndex(0)
	, m_NumPresentCalls(0)
	, m_NumAcquireCalls(0)
	, m_LockToVsync(lockToVsync)
	, m_PresentID(0)
{

	// 创建Surface
	VulkanPlatform::CreateSurface(instance, &m_Surface);

	// 设置Present Queue
	m_Device->SetupPresentQueue(m_Surface);

	// 遍历format找出合适的Format
	uint32 numFormats;
	VERIFYVULKANRESULT_EXPANDED(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFormats, nullptr));

	std::vector<VkSurfaceFormatKHR> formats(numFormats);
	VERIFYVULKANRESULT_EXPANDED(vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFormats, formats.data()));
	
    VkSurfaceFormatKHR currFormat = {};
	if (outPixelFormat != PF_Unknown)
	{
		bool bFound = false;
		if (G_PixelFormats[outPixelFormat].supported)
		{
			VkFormat requested = (VkFormat)G_PixelFormats[outPixelFormat].platformFormat;
			for (int32 index = 0; index < formats.size(); ++index)
			{
				if (formats[index].format == requested)
				{
					bFound     = true;
					currFormat = formats[index];
					break;
				}
			}

			if (!bFound)
			{
				MLOG("Requested PixelFormat %d not supported by this swapchain! Falling back to supported swapchain format...", (uint32)outPixelFormat);
				outPixelFormat = PF_Unknown;
			}
		}
		else
		{
			MLOG("Requested PixelFormat %d not supported by this Vulkan implementation!", (uint32)outPixelFormat);
			outPixelFormat = PF_Unknown;
		}
	}

	if (outPixelFormat == PF_Unknown)
	{
		for (int32 index = 0; index < formats.size(); ++index)
		{
			for (int32 pfIndex = 0; pfIndex < PF_MAX; ++pfIndex)
			{
				if (formats[index].format == G_PixelFormats[pfIndex].platformFormat)
				{
					outPixelFormat = (PixelFormat)pfIndex;
					currFormat     = formats[index];
					MLOG("No swapchain format requested, picking up VulkanFormat %d", (uint32)currFormat.format);
					break;
				}
			}
			
			if (outPixelFormat != PF_Unknown)
			{
				break;
			}
		}
	}
	
	if (outPixelFormat == PF_Unknown)
	{
		MLOG("Can't find a proper pixel format for the swapchain, trying to pick up the first available");
		VkFormat platformFormat = PixelFormatToVkFormat(outPixelFormat, false);
		bool supported = false;
		for (int32 index = 0; index < formats.size(); ++index)
		{
			if (formats[index].format == platformFormat)
			{
				supported  = true;
				currFormat = formats[index];
			}
		}
	}
	
	// 检测Present Model
    uint32 numFoundPresentModes = 0;
    VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFoundPresentModes, nullptr));
   
	std::vector< VkPresentModeKHR> foundPresentModes(numFoundPresentModes);
    VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(m_Device->GetPhysicalHandle(), m_Surface, &numFoundPresentModes, foundPresentModes.data()));
    
    bool foundPresentModeMailbox   = false;
    bool foundPresentModeImmediate = false;
    bool foundPresentModeFIFO      = false;
    
    MLOG("Found %d present mode.", numFoundPresentModes);
    for (int32 index = 0; index < numFoundPresentModes; ++index)
    {
        switch (foundPresentModes[index])
        {
            case VK_PRESENT_MODE_MAILBOX_KHR:
                foundPresentModeMailbox = true;
                MLOG("- VK_PRESENT_MODE_MAILBOX_KHR (%d)", (int32)VK_PRESENT_MODE_MAILBOX_KHR);
                break;
            case VK_PRESENT_MODE_IMMEDIATE_KHR:
                foundPresentModeImmediate = true;
                MLOG("- VK_PRESENT_MODE_IMMEDIATE_KHR (%d)", (int32)VK_PRESENT_MODE_IMMEDIATE_KHR);
                break;
            case VK_PRESENT_MODE_FIFO_KHR:
                foundPresentModeFIFO = true;
                MLOG("- VK_PRESENT_MODE_FIFO_KHR (%d)", (int32)VK_PRESENT_MODE_FIFO_KHR);
                break;
            default:
                MLOG("- VkPresentModeKHR (%d)", (int32)foundPresentModes[index]);
                break;
        }
    }
    
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    if (foundPresentModeImmediate && !m_LockToVsync)
    {
        presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    }
    else if (foundPresentModeMailbox)
    {
        presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    else if (foundPresentModeFIFO)
    {
        presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }
    else
    {
        MLOG("Couldn't find desired PresentMode! Using %d", (int32)foundPresentModes[0]);
        presentMode = foundPresentModes[0];
    }
    
    MLOG("Selected VkPresentModeKHR mode %d", presentMode);

	VkSurfaceCapabilitiesKHR surfProperties;
	VERIFYVULKANRESULT_EXPANDED(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->GetPhysicalHandle(), m_Surface, &surfProperties));
    
    MLOG("Surface minSize:%dx%d maxSize:%dx%d",
         (int32)surfProperties.minImageExtent.width, (int32)surfProperties.minImageExtent.height,
         (int32)surfProperties.maxImageExtent.width, (int32)surfProperties.maxImageExtent.height
    );
    
	VkSurfaceTransformFlagBitsKHR preTransform;
	if (surfProperties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = surfProperties.currentTransform;
	}

	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	if (surfProperties.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
	{
		compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	
	uint32 desiredNumBuffers = surfProperties.maxImageCount > 0 ? MMath::Clamp(*outDesiredNumBackBuffers, surfProperties.minImageCount, surfProperties.maxImageCount) : *outDesiredNumBackBuffers;
    uint32 sizeX = surfProperties.currentExtent.width  == 0xFFFFFFFF ? width : surfProperties.currentExtent.width;
	uint32 sizeY = surfProperties.currentExtent.height == 0xFFFFFFFF ? height : surfProperties.currentExtent.height;
	
	ZeroVulkanStruct(m_SwapChainInfo, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
	m_SwapChainInfo.surface				= m_Surface;
	m_SwapChainInfo.minImageCount		= desiredNumBuffers;
	m_SwapChainInfo.imageFormat			= currFormat.format;
	m_SwapChainInfo.imageColorSpace		= currFormat.colorSpace;
	m_SwapChainInfo.imageExtent.width	= sizeX;
	m_SwapChainInfo.imageExtent.height	= sizeY;
	m_SwapChainInfo.imageUsage			= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	m_SwapChainInfo.preTransform		= preTransform;
	m_SwapChainInfo.imageArrayLayers	= 1;
	m_SwapChainInfo.imageSharingMode	= VK_SHARING_MODE_EXCLUSIVE;
	m_SwapChainInfo.presentMode			= presentMode;
	m_SwapChainInfo.oldSwapchain		= VK_NULL_HANDLE;
	m_SwapChainInfo.clipped				= VK_TRUE;
	m_SwapChainInfo.compositeAlpha		= compositeAlpha;
	
	*outDesiredNumBackBuffers			= desiredNumBuffers;

	if (m_SwapChainInfo.imageExtent.width == 0)
	{
		m_SwapChainInfo.imageExtent.width = width;
	}
	if (m_SwapChainInfo.imageExtent.height == 0)
	{
		m_SwapChainInfo.imageExtent.height = height;
	}

	// 检测是否支持present
	VkBool32 supportsPresent;
	VERIFYVULKANRESULT(vkGetPhysicalDeviceSurfaceSupportKHR(m_Device->GetPhysicalHandle(), m_Device->GetPresentQueue()->GetFamilyIndex(), m_Surface, &supportsPresent));
    if (!supportsPresent)
    {
        MLOGE("Present queue not support.")
    }

	// 创建SwapChain
	VERIFYVULKANRESULT(vkCreateSwapchainKHR(m_Device->GetInstanceHandle(), &m_SwapChainInfo, VULKAN_CPU_ALLOCATOR, &m_SwapChain));

	// 获取Backbuffer数量
	uint32 numSwapChainImages;
	VERIFYVULKANRESULT(vkGetSwapchainImagesKHR(m_Device->GetInstanceHandle(), m_SwapChain, &numSwapChainImages, nullptr));
	
	outImages.resize(numSwapChainImages);
	VERIFYVULKANRESULT(vkGetSwapchainImagesKHR(m_Device->GetInstanceHandle(), m_SwapChain, &numSwapChainImages, outImages.data()));
    
	// 创建Fence
	m_ImageAcquiredSemaphore.resize(numSwapChainImages);
	for (int32 index = 0; index < numSwapChainImages; ++index)
	{
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		VERIFYVULKANRESULT(vkCreateSemaphore(m_Device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_ImageAcquiredSemaphore[index]));
	}
	
	m_PresentID = 0;
    MLOG("SwapChain: Backbuffer:%d Format:%d ColorSpace:%d Size:%dx%d Present:%d", m_SwapChainInfo.minImageCount, m_SwapChainInfo.imageFormat, m_SwapChainInfo.imageColorSpace, m_SwapChainInfo.imageExtent.width, m_SwapChainInfo.imageExtent.height, m_SwapChainInfo.presentMode);
}
```

## CommandPool以及CommandBuffers

Vulkan是比较底层的API，需要我们自己处理CPU跟GPU同步操作。在OpenGL或者其他中，是由驱动帮我们完成这些操作，我们在使用当中感觉不到这些操作是异步的。由于是异步操作，因此只能将我们的需求封装成一个一个的指令，然后发送一批或者几批指令到GPU，交给GPU执行，然后通过信号量的方式来进行两者之间的同步操作。以下是创建CommandPool的示例：

```c++
void VulkanRHI::CreateCommandPool()
{
    VkCommandPoolCreateInfo createInfo;
    ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
    createInfo.queueFamilyIndex = m_Device->GetGraphicsQueue()->GetFamilyIndex();
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VERIFYVULKANRESULT(vkCreateCommandPool(m_Device->GetInstanceHandle(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_CommandPool));
}
```

由于我们的交换链有多个缓冲区，每个缓冲区的可能会在不同的地方使用。例如：缓冲区0正在被显示器显示；缓冲区2已经被GPU填充正在等待被显示；缓冲区3则正在被GPU填充。因此如果我们只使用单一的CommandBuffer，那么就只能等待GPU执行完毕之后再次使用该CommandBuffer。因此为了最大化的利用GPU，我们至少选择创建于交换链缓冲区数量一致的CommandBuffer。

```c++
void VulkanRHI::CreateCommandBuffers()
{
    m_CommandBuffers.resize(m_FrameImageViews.size());

    VkCommandBufferAllocateInfo allocateInfo;
    ZeroVulkanStruct(allocateInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    allocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool        = m_CommandPool;
    allocateInfo.commandBufferCount = uint32_t(m_FrameImageViews.size());
    VERIFYVULKANRESULT(vkAllocateCommandBuffers(m_Device->GetInstanceHandle(), &allocateInfo, m_CommandBuffers.data()));
}
```

## Depth Stencil

为了进行深度以及模板测试，我们需要创建深度以及模板缓冲区，这个缓冲区全程只需要一个即可，因此这个缓冲区只需要GPU在绘制阶段使用。这个缓冲区是需要内存空间用来存储每一个像素的深度以及模板值的，因此我们不仅要创建对应的Image还需要分配内存。

```c++
void VulkanRHI::CreateDepthStencil()
{
    int32 width  = m_SwapChain->GetWidth();
    int32 height = m_SwapChain->GetHeight();
    
    VkImageCreateInfo imageCreateInfo;
    ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    imageCreateInfo.imageType   = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format      = PixelFormatToVkFormat(m_DepthFormat, false);
    imageCreateInfo.extent      = { (uint32)width, (uint32)height, 1 };
    imageCreateInfo.mipLevels   = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples     = m_SampleCount;
    imageCreateInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageCreateInfo.flags       = 0;
    VERIFYVULKANRESULT(vkCreateImage(m_Device->GetInstanceHandle(), &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilImage));
    
    VkImageViewCreateInfo imageViewCreateInfo;
    ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format   = PixelFormatToVkFormat(m_DepthFormat, false);
    imageViewCreateInfo.flags    = 0;
    imageViewCreateInfo.image    = m_DepthStencilImage;
    imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
    imageViewCreateInfo.subresourceRange.levelCount     = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount     = 1;
    
    VkMemoryRequirements memRequire;
    vkGetImageMemoryRequirements(m_Device->GetInstanceHandle(), imageViewCreateInfo.image, &memRequire);
    uint32 memoryTypeIndex = 0;
    VERIFYVULKANRESULT(m_Device->GetMemoryManager().GetMemoryTypeFromProperties(memRequire.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex));
    
    VkMemoryAllocateInfo memAllocateInfo;
    ZeroVulkanStruct(memAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    memAllocateInfo.allocationSize  = memRequire.size;
    memAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    
    vkAllocateMemory(m_Device->GetInstanceHandle(), &memAllocateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilMemory);
    vkBindImageMemory(m_Device->GetInstanceHandle(), m_DepthStencilImage, m_DepthStencilMemory, 0);
    VERIFYVULKANRESULT(vkCreateImageView(m_Device->GetInstanceHandle(), &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilView));
}
```

## RenderPass

在绘制的时候，我们还需要指定附件的相关信息。

```c++
void VulkanRHI::CreateRenderPass()
{
    std::vector<VkAttachmentDescription> attachments(2);
    // color attachment
    attachments[0].format		  = PixelFormatToVkFormat(m_PixelFormat, false);
    attachments[0].samples		  = m_SampleCount;
    attachments[0].loadOp		  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    // depth stencil attachment
    attachments[1].format         = PixelFormatToVkFormat(m_DepthFormat, false);
    attachments[1].samples        = m_SampleCount;
    attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout	  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference colorReference = { };
    colorReference.attachment = 0;
    colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthReference = { };
    depthReference.attachment = 1;
    depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    VkSubpassDescription subpassDescription = { };
    subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount    = 1;
    subpassDescription.pColorAttachments       = &colorReference;
    subpassDescription.pDepthStencilAttachment = &depthReference;
    subpassDescription.pResolveAttachments     = nullptr;
    subpassDescription.inputAttachmentCount    = 0;
    subpassDescription.pInputAttachments       = nullptr;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments    = nullptr;
    
    std::vector<VkSubpassDependency> dependencies(2);
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0;
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    dependencies[1].srcSubpass      = 0;
    dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    VkRenderPassCreateInfo renderPassInfo;
    ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpassDescription;
    renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies   = dependencies.data();
    VERIFYVULKANRESULT(vkCreateRenderPass(m_Device->GetInstanceHandle(), &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));
}
```

## FrameBuffer

RenderPass准备好之后，就可以创建出FrameBuffer。

```c++
void VulkanRHI::CreateFrameBuffer()
{
    int32 width  = m_SwapChain->GetWidth();
    int32 height = m_SwapChain->GetHeight();

	VkImageView attachments[2];
	attachments[1] = m_DepthStencilView;

	VkFramebufferCreateInfo frameBufferCreateInfo;
	ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
	frameBufferCreateInfo.renderPass      = m_RenderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments    = attachments;
	frameBufferCreateInfo.width			  = width;
	frameBufferCreateInfo.height		  = height;
	frameBufferCreateInfo.layers		  = 1;

	m_FrameBuffers.resize(m_FrameImageViews.size());
	for (uint32 i = 0; i < m_FrameBuffers.size(); ++i)
	{
		attachments[0] = m_FrameImageViews[i];
		VERIFYVULKANRESULT(vkCreateFramebuffer(m_Device->GetInstanceHandle(), &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
	}
}
```

## Pipeline

直到现在为止，我们只是准备好了最基本的信息。到了这一步我们才开始准备渲染所需的相关信息。首先是`Pipeline`，Pipeline跟我们使用的`材质`有点儿类似。我们需要说明整条渲染管线中顶点数据的输入类型格式；光栅化的方式；混合的方式；DepthStencil的行为以及参数；多重采样；使用哪几种Shader等等。

```c++
void CreatePipelines()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
		VkPipelineRasterizationStateCreateInfo rasterizationState;
		ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
		rasterizationState.polygonMode 			   = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode                = VK_CULL_MODE_NONE;
		rasterizationState.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable        = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable         = VK_FALSE;
		rasterizationState.lineWidth 			   = 1.0f;
        
		VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
        blendAttachmentState[0].colorWriteMask = (
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
        );
		blendAttachmentState[0].blendEnable = VK_FALSE;
        
		VkPipelineColorBlendStateCreateInfo colorBlendState;
		ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments    = blendAttachmentState;
        
		VkPipelineViewportStateCreateInfo viewportState;
		ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
		viewportState.viewportCount = 1;
		viewportState.scissorCount  = 1;
        
		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
		VkPipelineDynamicStateCreateInfo dynamicState;
		ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
		dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();
		dynamicState.pDynamicStates    = dynamicStateEnables.data();
        
		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
		depthStencilState.depthTestEnable 		= VK_TRUE;
		depthStencilState.depthWriteEnable 		= VK_TRUE;
		depthStencilState.depthCompareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp 			= VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp 			= VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp 		= VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable 	= VK_FALSE;
		depthStencilState.front 				= depthStencilState.back;

		VkPipelineMultisampleStateCreateInfo multisampleState;
		ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
		multisampleState.rasterizationSamples = m_VulkanRHI->GetSampleCount();
		multisampleState.pSampleMask 		  = nullptr;
		
		// (triangle.vert):
		// layout (location = 0) in vec3 inPos;
		// layout (location = 1) in vec3 inColor;
		// Attribute location 0: Position
		// Attribute location 1: Color
		// vertex input bindding
		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding   = 0; // Vertex Buffer 0
		vertexInputBinding.stride    = sizeof(Vertex); // Position + Color
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(2);
		// position
		vertexInputAttributs[0].binding  = 0;
        vertexInputAttributs[0].location = 0; // triangle.vert : layout (location = 0)
		vertexInputAttributs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[0].offset   = offsetof(Vertex, position);
		// color
		vertexInputAttributs[1].binding  = 0;
		vertexInputAttributs[1].location = 1; // triangle.vert : layout (location = 1)
		vertexInputAttributs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[1].offset   = offsetof(Vertex, color);
		
		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount   = 1;
		vertexInputState.pVertexBindingDescriptions      = &vertexInputBinding;
		vertexInputState.vertexAttributeDescriptionCount = 2;
		vertexInputState.pVertexAttributeDescriptions    = vertexInputAttributs.data();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
		ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = LoadSPIPVShader("assets/shaders/2_Triangle/triangle.vert.spv");
		shaderStages[0].pName  = "main";
		shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = LoadSPIPVShader("assets/shaders/2_Triangle/triangle.frag.spv");
		shaderStages[1].pName  = "main";
        
		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout 				= m_PipelineLayout;
		pipelineCreateInfo.renderPass 			= m_VulkanRHI->GetRenderPass();
		pipelineCreateInfo.stageCount 			= (uint32_t)shaderStages.size();
		pipelineCreateInfo.pStages 				= shaderStages.data();
		pipelineCreateInfo.pVertexInputState 	= &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState 	= &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState 	= &rasterizationState;
		pipelineCreateInfo.pColorBlendState 	= &colorBlendState;
		pipelineCreateInfo.pMultisampleState 	= &multisampleState;
		pipelineCreateInfo.pViewportState 		= &viewportState;
		pipelineCreateInfo.pDepthStencilState 	= &depthStencilState;
		pipelineCreateInfo.pDynamicState 		= &dynamicState;
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(m_Device, m_VulkanRHI->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));
		
		vkDestroyShaderModule(m_Device, shaderStages[0].module, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, shaderStages[1].module, VULKAN_CPU_ALLOCATOR);
	}
```

## VertexBuffer以及IndexBuffer

我们需要将顶点以及索引的数据传递给GPU并且存储到GPU内存中，以便GPU在每次显示时能够利用这些数据进行绘制。我们的数据需要从内存传递到共享内存，然后从共享内存传递到GPU内存。具体参考：http://xiaopengyou.fun/article/30。以下是VertexBuffer以及IndexBuffer的创建过程。

```c++
void CreateMeshBuffers()
	{
		// 顶点数据
		std::vector<Vertex> vertices = {
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};

		// 索引数据
		std::vector<uint16> indices = { 0, 1, 2 };
		m_IndicesCount = (uint32)indices.size();

		// 顶点数据以及索引数据在整个生命周期中几乎不会发生改变，因此最佳的方式是将这些数据存储到GPU的内存中。
		// 存储到GPU内存也能加快GPU的访问。为了存储到GPU内存中，需要如下几个步骤。
		// 1、在主机端(Host)创建一个Buffer
		// 2、将数据拷贝至该Buffer
		// 3、在GPU端(Local Device)创建一个Buffer
		// 4、通过Transfer簇将数据从主机端拷贝至GPU端
		// 5、删除主基端(Host)的Buffer
		// 6、使用GPU端(Local Device)的Buffer进行渲染
		VertexBuffer tempVertexBuffer;
		IndexBuffer  tempIndexBuffer;

		void* dataPtr = nullptr;
		VkMemoryRequirements memReqInfo;
		VkMemoryAllocateInfo memAllocInfo;
		ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

		// vertex buffer
		VkBufferCreateInfo vertexBufferInfo;
		ZeroVulkanStruct(vertexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		vertexBufferInfo.size  = vertices.size() * sizeof(Vertex);
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.buffer));

		vkGetBufferMemoryRequirements(m_Device, tempVertexBuffer.buffer, &memReqInfo);
		uint32 memoryTypeIndex = 0;
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, tempVertexBuffer.buffer, tempVertexBuffer.memory, 0));

		VERIFYVULKANRESULT(vkMapMemory(m_Device, tempVertexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, vertices.data(), vertexBufferInfo.size);
		vkUnmapMemory(m_Device, tempVertexBuffer.memory);

		// local device vertex buffer
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.buffer));

		vkGetBufferMemoryRequirements(m_Device, m_VertexBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_VertexBuffer.buffer, m_VertexBuffer.memory, 0));

		// index buffer
		VkBufferCreateInfo indexBufferInfo;
		ZeroVulkanStruct(indexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		indexBufferInfo.size  = m_IndicesCount * sizeof(uint16);
		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.buffer));

		vkGetBufferMemoryRequirements(m_Device, tempIndexBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, tempIndexBuffer.buffer, tempIndexBuffer.memory, 0));

		VERIFYVULKANRESULT(vkMapMemory(m_Device, tempIndexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, indices.data(), indexBufferInfo.size);
		vkUnmapMemory(m_Device, tempIndexBuffer.memory);
		
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.buffer));
		
		vkGetBufferMemoryRequirements(m_Device, m_IndicesBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_IndicesBuffer.buffer, m_IndicesBuffer.memory, 0));

		VkCommandBuffer xferCmdBuffer;
		// gfx queue自带transfer功能，为了优化需要使用转悠的xfer queue。这里为了简单，先将就用。
		VkCommandBufferAllocateInfo xferCmdBufferInfo;
		ZeroVulkanStruct(xferCmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
		xferCmdBufferInfo.commandPool        = m_VulkanRHI->GetCommandPool();
		xferCmdBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		xferCmdBufferInfo.commandBufferCount = 1;
		VERIFYVULKANRESULT(vkAllocateCommandBuffers(m_Device, &xferCmdBufferInfo, &xferCmdBuffer));

		// 开始录制命令
		VkCommandBufferBeginInfo cmdBufferBeginInfo;
		ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(xferCmdBuffer, &cmdBufferBeginInfo));
		
		VkBufferCopy copyRegion = {};
		copyRegion.size = vertices.size() * sizeof(Vertex);
		vkCmdCopyBuffer(xferCmdBuffer, tempVertexBuffer.buffer, m_VertexBuffer.buffer, 1, &copyRegion);
		
		copyRegion.size = indices.size() * sizeof(uint16);
		vkCmdCopyBuffer(xferCmdBuffer, tempIndexBuffer.buffer, m_IndicesBuffer.buffer, 1, &copyRegion);

		// 结束录制
		VERIFYVULKANRESULT(vkEndCommandBuffer(xferCmdBuffer));
		
		// 提交命令，并且等待命令执行完毕。
		VkSubmitInfo submitInfo;
		ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &xferCmdBuffer;

		VkFenceCreateInfo fenceInfo;
		ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceInfo.flags = 0;

		VkFence fence = VK_NULL_HANDLE;
		VERIFYVULKANRESULT(vkCreateFence(m_Device, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));
		VERIFYVULKANRESULT(vkQueueSubmit(m_VulkanRHI->GetDevice()->GetGraphicsQueue()->GetHandle(), 1, &submitInfo, fence));
		VERIFYVULKANRESULT(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, MAX_int64));

		vkDestroyFence(m_Device, fence, VULKAN_CPU_ALLOCATOR);
		vkFreeCommandBuffers(m_Device, m_VulkanRHI->GetCommandPool(), 1, &xferCmdBuffer);

		vkDestroyBuffer(m_Device, tempVertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, tempVertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(m_Device, tempIndexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, tempIndexBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}
```

## 录制命令

之前提到由于是异步操作，需要将命令打包上传给GPU，交给GPU执行。在Vulkan中有一个录制的概念，其实在DX12中也仍然是录制的概念。命令在录制过程中不允许其它操作，例如提交等。我们的一系列操作都会录制在CommandBuffer中。

```c++
void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color        = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass      = m_VulkanRHI->GetRenderPass();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues    = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width  = m_VulkanRHI->GetSwapChain()->GetWidth();
        renderPassBeginInfo.renderArea.extent.height = m_VulkanRHI->GetSwapChain()->GetHeight();
        
		std::vector<VkCommandBuffer>& drawCmdBuffers = m_VulkanRHI->GetCommandBuffers();
		std::vector<VkFramebuffer> frameBuffers      = m_VulkanRHI->GetFrameBuffers();
		for (int32 i = 0; i < drawCmdBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VkViewport viewport = {};
            viewport.width    = (float)renderPassBeginInfo.renderArea.extent.width;
            viewport.height   = (float)renderPassBeginInfo.renderArea.extent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
            
			VkRect2D scissor = {};
            scissor.extent.width  = (uint32)viewport.width;
            scissor.extent.height = (uint32)viewport.height;
			scissor.offset.x      = 0;
			scissor.offset.y      = 0;

			VkDeviceSize offsets[1] = { 0 };
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
			vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, &m_VertexBuffer.buffer, offsets);
			vkCmdBindIndexBuffer(drawCmdBuffers[i], m_IndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(drawCmdBuffers[i], m_IndicesCount, 1, 0, 0, 0);
			vkCmdEndRenderPass(drawCmdBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
		}
	}
```

录制完成之后，我们就可以将CommandBuffer提交给GPU执行。虽然可以提交给GPU执行，但是我们无法知道GPU什么时候执行完成，因此我们需要信号量来进行同步操作。

## 信号量

```c++
void CreateSemaphores()
	{
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		vkCreateSemaphore(m_Device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_PresentComplete);
		vkCreateSemaphore(m_Device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
	}
```

```c++
void CreateFences()
	{
		m_Fences.resize(m_VulkanRHI->GetSwapChain()->GetBackBufferCount());
		VkFenceCreateInfo fenceCreateInfo;
		ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (int32 i = 0; i < m_Fences.size(); ++i) 
		{
			VERIFYVULKANRESULT(vkCreateFence(m_Device, &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
		}
	}
```

## 提交命令

一切准备就绪之后，我们就可以将命令提交给GPU，交给GPU执行。

```c++
void Draw()
	{
		UpdateUniformBuffers();

		VkSwapchainKHR swapchain = m_VulkanRHI->GetSwapChain()->GetInstanceHandle();
		VkPipelineStageFlags waitStageMask = m_VulkanRHI->GetStageMask();
		std::vector<VkCommandBuffer>& drawCmdBuffers = m_VulkanRHI->GetCommandBuffers();
        
		// 请求一个空闲的Backbuffer，这里会一直同步直到Present引擎交出一个。
		VERIFYVULKANRESULT(vkAcquireNextImageKHR(m_Device, swapchain, UINT64_MAX, m_PresentComplete, (VkFence)nullptr, &m_CurrentBackBuffer));
		// 继续同步等待，所有提交的指令执行完毕。
		VERIFYVULKANRESULT(vkWaitForFences(m_Device, 1, &m_Fences[m_CurrentBackBuffer], VK_TRUE, UINT64_MAX));
		VERIFYVULKANRESULT(vkResetFences(m_Device, 1, &m_Fences[m_CurrentBackBuffer]));
		
		VkSubmitInfo submitInfo = {};
		submitInfo.sType 				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask 	= &waitStageMask;									
		submitInfo.pWaitSemaphores 		= &m_PresentComplete;
		submitInfo.waitSemaphoreCount 	= 1;																														
		submitInfo.pSignalSemaphores 	= &m_RenderComplete;
		submitInfo.signalSemaphoreCount = 1;											
		submitInfo.pCommandBuffers 		= &drawCmdBuffers[m_CurrentBackBuffer];
		submitInfo.commandBufferCount 	= 1;												
		
		// 提交绘制命令
		VERIFYVULKANRESULT(vkQueueSubmit(m_VulkanRHI->GetDevice()->GetGraphicsQueue()->GetHandle(), 1, &submitInfo, m_Fences[m_CurrentBackBuffer]));

		VkPresentInfoKHR presentInfo = {};
		ZeroVulkanStruct(presentInfo, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
		presentInfo.swapchainCount 	   = 1;
		presentInfo.pSwapchains 	   = &swapchain;
		presentInfo.pImageIndices 	   = &m_CurrentBackBuffer;
		presentInfo.pWaitSemaphores    = &m_RenderComplete;
		presentInfo.waitSemaphoreCount = 1;

		// 提交Present命令
		vkQueuePresentKHR(m_VulkanRHI->GetDevice()->GetPresentQueue()->GetHandle(), &presentInfo);
	}
```

## 最后说明

在这里我并没有详细说明每一步，只是捡了最重要的几个步骤来加以说明。其中大部分都是大段大段的复制粘贴代码，其原因是开头我给了两个非常详细的教程地址。他们将绘制三角形这个过程分解成了30多篇文章，详细的讲解每一个步骤以及作用，其实参考这些教程即可。从后面开始，我将会着重详细说明其中功能步骤。



















