#include "VulkanRHI.h"
#include "VulkanPlatform.h"

#if MONKEY_DEBUG

#define VK_CREATE_DEBUG_REPORT_CALLBACK_EXT_NAME  "vkCreateDebugUtilsMessengerEXT"
#define VK_DESTORY_DEBUG_REPORT_CALLBACK_EXT_NAME "vkDestroyDebugUtilsMessengerEXT"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData
)
{
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) 
	{
		MLOG("[%d][%s]:[%s]", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) 
	{
		MLOG("[%d][%s]:[%s]", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) 
	{
		MLOG("[%d][%s]:[%s]", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) 
	{
		MLOG("[%d][%s]:[%s]", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
	}

	return VK_FALSE;
}

void VulkanRHI::SetupDebugLayerCallback()
{
    VkDebugUtilsMessengerCreateInfoEXT debugInfo;
    ZeroVulkanStruct(debugInfo, VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
    debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugInfo.pfnUserCallback = DebugUtilsMessengerCallback;
    
    auto func    = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, VK_CREATE_DEBUG_REPORT_CALLBACK_EXT_NAME);
    bool success = true;
    if (func != nullptr) {
        success = func(m_Instance, &debugInfo, VULKAN_CPU_ALLOCATOR, &m_MsgCallback) == VK_SUCCESS;
    }
    else {
        success = false;
    }
    
    if (success) {
        MLOG("Setup debug callback success.");
    }
    else {
        MLOG("Setup debug callback failed.")
    }
}

void VulkanRHI::RemoveDebugLayerCallback()
{
    if (m_MsgCallback != VK_NULL_HANDLE)
    {
        PFN_vkDestroyDebugUtilsMessengerEXT destroyMsgCallback = (PFN_vkDestroyDebugUtilsMessengerEXT)(void*)vkGetInstanceProcAddr(m_Instance, VK_DESTORY_DEBUG_REPORT_CALLBACK_EXT_NAME);
        destroyMsgCallback(m_Instance, m_MsgCallback, VULKAN_CPU_ALLOCATOR);
    }
}

#endif