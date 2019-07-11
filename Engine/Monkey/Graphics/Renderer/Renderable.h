#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Core/MObject.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanResources.h"

#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Data/IndexBuffer.h"

#include <memory>

class Renderable : public MObject
{
public:

    Renderable(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer);
    
    virtual ~Renderable();
    
    inline std::shared_ptr<VertexBuffer> GetVertexBuffer() const
    {
        return m_VertexBuffer;
    }
    
    inline std::shared_ptr<IndexBuffer> GetIndexBuffer() const
    {
        return m_IndexBuffer;
    }

protected:
    
    std::shared_ptr<VertexBuffer>   m_VertexBuffer;
    std::shared_ptr<IndexBuffer>    m_IndexBuffer;

    VkDeviceSize                    m_VertexOffset;
};
