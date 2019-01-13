#include "VulkanRHI.h"
#include "VulkanPlatform.h"
#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/SlateApplication.h"

VulkanRHI::VulkanRHI()
	: m_Instance(VK_NULL_HANDLE)
	, m_Device(VK_NULL_HANDLE)
{

}

VulkanRHI::~VulkanRHI()
{

}

void VulkanRHI::Init()
{
	if (!VulkanPlatform::LoadVulkanLibrary()) {
		MLOG("%s\n", "Failed load vulkan libarary.");
		return;
	}

	InitInstance();
}

void VulkanRHI::PostInit()
{

}

void VulkanRHI::Shutdown()
{

}

void VulkanRHI::InitInstance()
{
	CreateInstance();
}

void VulkanRHI::RecreateSwapChain(void* newNativeWindow)
{

}

void VulkanRHI::SavePipelineCache()
{

}

void VulkanRHI::PooledUniformBuffersBeginFrame()
{

}

void VulkanRHI::ReleasePooledUniformBuffers()
{

}

void VulkanRHI::CreateInstance()
{
	GetInstanceLayersAndExtensions(m_InstanceExtensions, m_InstanceLayers, m_SupportsDebugUtilsExt);

	VkApplicationInfo appInfo;
	ZeroVulkanStruct(appInfo, VK_STRUCTURE_TYPE_APPLICATION_INFO);
	appInfo.pApplicationName = SlateApplication::Get().GetPlatformApplication()->GetWindow()->GetTitle().c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo instanceCreateInfo;
	ZeroVulkanStruct(instanceCreateInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = m_InstanceExtensions.size();
	instanceCreateInfo.ppEnabledExtensionNames = m_InstanceExtensions.size() > 0 ? m_InstanceExtensions.data() : nullptr;
	instanceCreateInfo.enabledLayerCount = m_InstanceLayers.size();
	instanceCreateInfo.ppEnabledLayerNames = m_InstanceLayers.size() > 0 ? m_InstanceLayers.data() : nullptr;

	VkResult result = vkCreateInstance(&instanceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Instance);

	if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		MLOG("%s" "Cannot find a compatible Vulkan driver (ICD).");
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
}

void VulkanRHI::SelectAndInitDevice()
{

}

void VulkanRHI::InitGPU(VkDevice device)
{

}

void VulkanRHI::InitDevice(VkDevice device)
{

}
