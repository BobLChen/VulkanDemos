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

void VulkanRHI::GetInstanceLayersAndExtensions(std::vector<const char*>& outInstanceExtensions, std::vector<const char*>& outInstanceLayers, bool& outDebugUtils)
{

}

void VulkanRHI::CreateInstance()
{
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
