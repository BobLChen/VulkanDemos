#include "Renderable.h"

Renderable::Renderable(std::shared_ptr<VertexBuffer> vertexBuffer, std::shared_ptr<IndexBuffer> indexBuffer)
    : m_VertexBuffer(vertexBuffer)
    , m_IndexBuffer(indexBuffer)
    , m_VertexOffset(0)
{
    
}

Renderable::~Renderable()
{
    m_VertexBuffer = nullptr;
    m_IndexBuffer  = nullptr;
}
