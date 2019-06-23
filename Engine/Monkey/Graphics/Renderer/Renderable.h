#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanResources.h"
#include "Core/MObject.h"
#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Data/IndexBuffer.h"

#include <memory>

class Renderable : public MObject
{
public:
    Renderable(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer);
    
    virtual ~Renderable();
    
    FORCEINLINE void BindDrawToCommand(VkCommandBuffer command) const
    {
        vkCmdDrawIndexed(command, m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
    }
    
    FORCEINLINE void BindBufferToCommand(VkCommandBuffer command) const
    {
        vkCmdBindVertexBuffers(command, 0, (uint32_t)m_VertexBuffer->GetVKBuffers().size(), m_VertexBuffer->GetVKBuffers().data(), &m_VertexOffset);
        vkCmdBindIndexBuffer(command, m_IndexBuffer->GetBuffer(), 0, m_IndexBuffer->GetIndexType());
    }
    
    FORCEINLINE std::shared_ptr<VertexBuffer> GetVertexBuffer() const
    {
        return m_VertexBuffer;
    }
    
    FORCEINLINE std::shared_ptr<IndexBuffer> GetIndexBuffer() const
    {
        return m_IndexBuffer;
    }
protected:
    
    std::shared_ptr<VertexBuffer>   m_VertexBuffer;
    std::shared_ptr<IndexBuffer>    m_IndexBuffer;
    VkDeviceSize                    m_VertexOffset;
};
