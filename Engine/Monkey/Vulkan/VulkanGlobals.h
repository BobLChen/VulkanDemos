#pragma once

#include <memory>
#include "Common/Log.h"

#define VULKAN_CPU_ALLOCATOR nullptr

#define VERIFYVULKANRESULT(VkFunction)				{ const VkResult ScopedResult = VkFunction; if (ScopedResult != VK_SUCCESS) { MLOG("VKResult=%d,Function=%d,Line=%d,Line=%d", ScopedResult, #VkFunction, __FILE__, __LINE__); }}
#define VERIFYVULKANRESULT_EXPANDED(VkFunction)		{ const VkResult ScopedResult = VkFunction; if (ScopedResult < VK_SUCCESS) { MLOG("VKResult=%d,Function=%d,Line=%d,Line=%d", ScopedResult, #VkFunction, __FILE__, __LINE__); }}

template<class T>
static FORCEINLINE void ZeroVulkanStruct(T& vkStruct, VkStructureType vkType)
{
	vkStruct.sType = vkType;
	memset(((uint8*)&vkStruct) + sizeof(VkStructureType), 0, sizeof(T) - sizeof(VkStructureType));
}
