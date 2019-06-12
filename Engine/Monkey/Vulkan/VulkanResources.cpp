#include "VulkanResources.h"

#include "Utils/Crc.h"

void VertexInputDeclareInfo::GenerateHash()
{
    if (m_Invalid)
    {
        m_hash  = 0;
        m_hash  = Crc::MemCrc32(m_Bindings.data(), int32(m_Bindings.size() * sizeof(BindingDescription)));
        m_hash  = Crc::MemCrc32(m_InputAttributes.data(), int32(m_InputAttributes.size() * sizeof(AttributeDescription)), m_hash);
        m_Invalid = false;
    }
}
