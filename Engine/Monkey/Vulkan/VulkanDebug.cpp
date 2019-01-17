#include "VulkanRHI.h"

#define VK_DESTORY_DEBUG_REPORT_CALLBACK_EXT_NAME "vkDestroyDebugReportCallbackEXT"
#define VK_CREATE_DEBUG_REPORT_CALLBACK_EXT_NAME  "vkCreateDebugReportCallbackEXT"

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallBack(
                                                          VkDebugReportFlagsEXT flags,
                                                          VkDebugReportObjectTypeEXT objType,
                                                          uint64_t obj,
                                                          size_t location,
                                                          int32_t code,
                                                          const char* layerPrefix,
                                                          const char* msg,
                                                          void* userData)
{
    printf("ERROR -> LayerPrefix:%s Msg:%s\n", layerPrefix, msg);
//    // Select prefix depending on flags passed to the callback
//    // Note that multiple flags may be set for a single validation message
//    std::string prefix("");
//
//    // Error that may result in undefined behaviour
//    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
//    {
//        prefix += "ERROR:";
//    }
//
//    // Warnings may hint at unexpected / non-spec API usage
//    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
//    {
//        prefix += "WARNING:";
//    }
//
//    // May indicate sub-optimal usage of the API
//    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
//    {
//        prefix += "PERFORMANCE:";
//    }
//
//    // Informal messages that may become handy during debugging
//    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
//    {
//        prefix += "INFO:";
//    }
//
//    // Diagnostic info from the Vulkan loader and layers
//    // Usually not helpful in terms of API usage, but may help to debug layer and loader problems
//    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
//    {
//        prefix += "DEBUG:";
//    }
//
//    MLOG("%s [%s] Code %d : %s", prefix.c_str(), pLayerPrefix, msgCode, pMsg);
    return VK_FALSE;
}

void VulkanRHI::SetupDebugLayerCallback()
{
    VkDebugReportCallbackCreateInfoEXT debugInfo = {};
    debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debugInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    debugInfo.pfnCallback = VulkanDebugCallBack;
    debugInfo.pUserData = this;
    
    auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, VK_CREATE_DEBUG_REPORT_CALLBACK_EXT_NAME);
    
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

void VulkanRHI::RemoveDebugLayerCallback()
{
    if (m_MsgCallback != VK_NULL_HANDLE)
    {
        PFN_vkDestroyDebugReportCallbackEXT destroyMsgCallback = (PFN_vkDestroyDebugReportCallbackEXT)(void*)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugReportCallbackEXT");
        destroyMsgCallback(m_Instance, m_MsgCallback, nullptr);
    }
}
