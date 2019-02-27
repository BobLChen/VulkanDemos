#include "VertexBuffer.h"

VertexData::VertexData()
    : m_VertexSize(0)
    , m_VertexCount(0)
    , m_Data(nullptr)
    , m_DataSize(0)
    , m_CurrentChannels(0)
{
    
}

VertexData::~VertexData()
{
    if (m_Data) {
        delete[] m_Data;
    }
    m_Data = nullptr;
}
