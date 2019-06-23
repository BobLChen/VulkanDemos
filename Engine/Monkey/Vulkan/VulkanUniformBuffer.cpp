#include "Graphics/Shader/Shader.h"

#include "VulkanPipeline.h"
#include "VulkanDescriptorInfo.h"
#include "VulkanResources.h"
#include "Engine.h"

VulkanUniformBuffer::VulkanUniformBuffer(const UniformBufferLayout& inLayout, const void* contents, UniformBufferUsage inUsage)
    : UniformBuffer(inLayout)
{
    if (inLayout.constantBufferSize > 0)
    {
        constantData.resize(inLayout.constantBufferSize);
        if (contents)
        {
            memcpy(constantData.data(), contents, inLayout.constantBufferSize);
        }
    }
}

void VulkanUniformBuffer::UpdateConstantData(const void* contents, int32 contentsSize)
{
    memcpy(constantData.data(), contents, contentsSize);
}