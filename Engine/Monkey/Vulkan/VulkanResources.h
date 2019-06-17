#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/RHIDefinitions.h"
#include "Vulkan/VulkanPlatform.h"

#include "Utils/Crc.h"

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
    
    inline bool operator == (const VertexStreamInfo& rhs) const
    {
        return channelMask == rhs.channelMask && size == rhs.size && alignment == rhs.alignment && allocationSize == rhs.allocationSize;
    }
    
    inline bool operator != (const VertexStreamInfo& rhs) const
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
    
    inline bool operator == (const VertexChannelInfo& rhs) const
    {
        return stream == rhs.stream && offset == rhs.offset && format == rhs.format && attribute == rhs.attribute;
    }
    
    inline bool operator != (const VertexChannelInfo& rhs) const
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
    
    inline void AddBinding(const BindingDescription& binding)
    {
        m_Invalid = true;
        m_Bindings.push_back(binding);
    }
    
    inline void AddAttribute(const AttributeDescription& attribute)
    {
        m_Invalid = true;
        m_InputAttributes.push_back(attribute);
    }
    
    inline uint32 GetHash() const
    {
        return m_hash;
    }
    
    inline const std::vector<BindingDescription>& GetBindings() const
    {
        return m_Bindings;
    }
    
    inline const std::vector<AttributeDescription>& GetAttributes() const
    {
        return m_InputAttributes;
    }

	void Clear()
	{
		m_Bindings.clear();
		m_InputAttributes.clear();
		m_Invalid = true;
		m_hash    = 0;
	}
    
    void GenerateHash();
    
protected:
    bool      m_Invalid;
    uint32    m_hash;
    
    std::vector<BindingDescription>      m_Bindings;
    std::vector<AttributeDescription>    m_InputAttributes;
};

class VertexInputBindingInfo
{
public:
    VertexInputBindingInfo()
    : m_Invalid(true)
    , m_Hash(0)
    {
        
    }
    
    inline int32 GetLocation(VertexAttribute attribute) const
    {
        for (int32 i = 0; i < m_Attributes.size(); ++i)
        {
            if (m_Attributes[i] == attribute)
            {
                return m_Locations[i];
            }
        }
        
        MLOGE("Can't found location, Attribute : %d", attribute);
        return -1;
    }
    
    inline uint32 GetHash() const
    {
        return m_Hash;
    }
    
    inline void AddBinding(VertexAttribute attribute, int32 location)
    {
        m_Invalid = true;
        m_Attributes.push_back(attribute);
        m_Locations.push_back(location);
    }
    
    inline int32 GetInputCount() const
    {
        return int32(m_Attributes.size());
    }
    
    inline void Clear()
    {
        m_Invalid = false;
        m_Hash    = 0;
        m_Attributes.clear();
        m_Locations.clear();
    }
    
    inline void GenerateHash()
    {
        if (m_Invalid)
        {
            m_Hash = Crc::MemCrc32(m_Attributes.data(), int32(m_Attributes.size() * sizeof(int32)), 0);
            m_Hash = Crc::MemCrc32(m_Locations.data(), int32(m_Locations.size() * sizeof(int32)), m_Hash);
            m_Invalid = false;
        }
    }
    
    inline const std::vector<VertexAttribute>& GetAttributes() const
    {
        return m_Attributes;
    }
    
protected:
    bool                         m_Invalid;
    uint32                       m_Hash;
    std::vector<VertexAttribute> m_Attributes;
    std::vector<int32>           m_Locations;
};