#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/RHIDefinitions.h"
#include "Vulkan/VulkanPlatform.h"

#include <vector>

struct VertexStreamInfo
{
    uint32 channelMask;
    uint32 size;
    uint32 alignment;
    uint32 allocationSize;
    
    VertexStreamInfo()
        : channelMask(0)
        , size(0)
        , alignment(0)
        , allocationSize(0)
    {
        
    }
    
    FORCEINLINE bool operator == (const VertexStreamInfo& rhs) const
    {
        return channelMask == rhs.channelMask && size == rhs.size && alignment == rhs.alignment && allocationSize == rhs.allocationSize;
    }
    
    FORCEINLINE bool operator != (const VertexStreamInfo& rhs) const
    {
        return !(*this == rhs);
    }
};

struct VertexChannelInfo
{
    uint8             stream;
    uint8             offset;
    VertexElementType format;
    VertexAttribute   attribute;
    
    VertexChannelInfo()
        : stream(0)
        , offset(0)
        , format(VertexElementType::VET_None)
        , attribute(VertexAttribute::VA_None)
    {
        
    }
    
    FORCEINLINE bool operator == (const VertexChannelInfo& rhs) const
    {
        return stream == rhs.stream && offset == rhs.offset && format == rhs.format && attribute == rhs.attribute;
    }
    
    FORCEINLINE bool operator != (const VertexChannelInfo& rhs) const
    {
        return !(*this == rhs);
    }
};

class VertexInputDeclareInfo
{
public:
    struct BindingDescription
    {
        uint32_t            binding;
        uint32_t            stride;
        VkVertexInputRate   inputRate;
    };
    
    struct AttributeDescription
    {
        uint32_t            binding;
        VkFormat            format;
        uint32_t            offset;
        VertexAttribute     attribute;
    };
    
public:
    VertexInputDeclareInfo()
        : m_Invalid(true)
        , m_hash(0)
    {
        
    }
    
    FORCEINLINE void AddBinding(const BindingDescription& binding)
    {
        m_Invalid = true;
        m_Bindings.push_back(binding);
    }
    
    FORCEINLINE void AddAttribute(const AttributeDescription& attribute)
    {
        m_Invalid = true;
        m_InputAttributes.push_back(attribute);
    }
    
    FORCEINLINE uint32 GetHash() const
    {
        return m_hash;
    }
    
    FORCEINLINE const std::vector<BindingDescription>& GetBindings() const
    {
        return m_Bindings;
    }
    
    FORCEINLINE const std::vector<AttributeDescription>& GetAttributes() const
    {
        return m_InputAttributes;
    }
    
    void GenerateHash();
    
protected:
    bool      m_Invalid;
    uint32    m_hash;
    
    std::vector<BindingDescription>      m_Bindings;
    std::vector<AttributeDescription>    m_InputAttributes;
};
