#pragma once

#include <memory>
#include <string.h>

#include "Common/Log.h"

#define VULKAN_CPU_ALLOCATOR nullptr

#define VERIFYVULKANRESULT(VkFunction)				{ const VkResult scopedResult = VkFunction; if (scopedResult != VK_SUCCESS) { MLOGE("VKResult=%d,Function=%s,File=%s,Line=%d", scopedResult, #VkFunction, __FILE__, __LINE__); }}
#define VERIFYVULKANRESULT_EXPANDED(VkFunction)		{ const VkResult scopedResult = VkFunction; if (scopedResult < VK_SUCCESS)  { MLOGE("VKResult=%d,Function=%s,File=%s,Line=%d", scopedResult, #VkFunction, __FILE__, __LINE__); }}

template<class T>
static FORCEINLINE void ZeroVulkanStruct(T& vkStruct, VkStructureType vkType)
{
	vkStruct.sType = vkType;
	memset(((uint8*)&vkStruct) + sizeof(VkStructureType), 0, sizeof(T) - sizeof(VkStructureType));
}