#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/RHIDefinitions.h"
#include "Vulkan/VulkanPlatform.h"

#include "Utils/Crc.h"
#include "Math/Math.h"
#include "Utils/Alignment.h"
#include "HAL/ThreadSafeCounter.h"

#include <vector>

class VulkanDevice;
class VulkanSubBufferAllocator;
class VulkanBufferSubAllocation;

class MResource
{
public:

    MResource(bool inDoNotDeferDelete = true)
        : m_MarkedForDelete(0)
        , m_DoNotDeferDelete(inDoNotDeferDelete)
    {

    }

    virtual ~MResource()
    {
        if (m_NumRef.GetValue() != 0)
        {
            MLOGE("Can't release resources.");
        }
    }

    uint32 AddRef() const
    {
        int32 newValue = m_NumRef.Increment();
        return newValue;
    }

    uint32 Release() const
    {
        int32 newValue = m_NumRef.Decrement();
        if (newValue == 0)
        {
            if (!DeferDelete())
            {
                delete this;
            }
            else 
            {
                m_MarkedForDelete = true;
                m_PendingDeletes.push_back(const_cast<MResource*>(this));
            }
        }
        return newValue;
    }

    inline uint32 GetRefCount() const
    {
        int32 value = m_NumRef.GetValue();
        return value;
    }

    inline bool DeferDelete() const
    {
        return !m_DoNotDeferDelete;
    }


    inline bool IsValid() const
    {
        return !m_MarkedForDelete && m_NumRef.GetValue() > 0;
    }

private:

    struct DeferDeleteResources
    {
        DeferDeleteResources(uint32 inFrameDeleted = 0)
            : frameDeleted(inFrameDeleted)
        {

        }

        std::vector<MResource*> resources;
        uint32                  frameDeleted;
    };

    static std::vector<DeferDeleteResources>    g_DeferredDeletionQueue;
    static uint32                               g_CurrentFrame;

    static std::vector<MResource*>              m_PendingDeletes;

private:

    mutable int32               m_MarkedForDelete;
    mutable ThreadSafeCounter   m_NumRef;

    bool                        m_DoNotDeferDelete;
};

struct UniformBufferLayout
{
public:

    struct ResourceParameter
    {
        uint16                  memberOffset;
        UniformBufferBaseType   memberType;
    };

    uint32                          constantBufferSize;
    std::vector<ResourceParameter>  resources;

public:

    explicit UniformBufferLayout(std::string inName)
        : constantBufferSize(0)
        , m_Name(inName)
        , m_Hash(0)
    {
        
    }

    explicit UniformBufferLayout()
        : constantBufferSize(0)
        , m_Name("")
        , m_Hash(0)
    {

    }

    inline uint32 GetHash() const
    {
        return m_Hash;
    }

    void ComputeHash()
	{
		uint32 tempHash = constantBufferSize << 16;
		
		for (int32 i = 0; i < resources.size(); ++i)
		{
			tempHash ^= resources[i].memberOffset;
		}

		uint32 num = resources.size();
		
		while (num >= 4)
		{
			tempHash ^= (resources[--num].memberType << 0);
			tempHash ^= (resources[--num].memberType << 8);
			tempHash ^= (resources[--num].memberType << 16);
			tempHash ^= (resources[--num].memberType << 24);
		}

		while (num >= 2)
		{
			tempHash ^= resources[--num].memberType << 0;
			tempHash ^= resources[--num].memberType << 16;
		}

		while (num > 0)
		{
			tempHash ^= resources[--num].memberType;
		}
        
		m_Hash = tempHash;
	}

    void CopyFrom(const UniformBufferLayout& source)
    {
        constantBufferSize  = source.constantBufferSize;
        resources           = source.resources;
        m_Name              = source.m_Name;
        m_Hash              = source.m_Hash;
    }

    inline const std::string& GetName() const
    {
        return m_Name;
    }

private:
    std::string     m_Name;
    uint32          m_Hash;
};

class UniformBuffer : public MResource
{
public:

    UniformBuffer()
    {

    }

public:

};

// not real uniform buffer
class VulkanUniformBuffer : public UniformBuffer
{
public:

    VulkanUniformBuffer(uint32 contentSize);

    void UpdateConstantData(const void* contents, int32 contentsSize);

	inline const uint32 GetContentSize() const
	{
		return constantData.size();
	}

	inline const uint8* GetData() const
	{
		return constantData.data();
	}

protected:
    std::vector<uint8> constantData;
};

// ring buffer
class VulkanRingBuffer
{
public:
	VulkanRingBuffer(VulkanDevice* device, uint64 totalSize, VkFlags usage, VkMemoryPropertyFlags memPropertyFlags);

	virtual ~VulkanRingBuffer();

	inline uint64 AllocateMemory(uint64 size, uint32 alignment)
	{
		alignment = MMath::Max(alignment, m_MinAlignment);
		uint64 allocationOffset = Align<uint64>(m_BufferOffset, alignment);
		if (allocationOffset + size <= m_BufferSize)
		{
			m_BufferOffset = allocationOffset + size;
			return allocationOffset;
		}

		return WrapAroundAllocateMemory(size, alignment);
	}

	VulkanSubBufferAllocator* GetBufferAllocator() const;

	uint32 GetBufferOffset() const;

	VkBuffer GetHandle() const;

	void* GetMappedPointer();

protected:
	uint64 WrapAroundAllocateMemory(uint64 size, uint32 alignment);

protected:
	VulkanDevice*				m_VulkanDevice;

	uint64						m_BufferSize;
	uint64						m_BufferOffset;
	uint32						m_MinAlignment;
	VulkanBufferSubAllocation*	m_BufferSubAllocation;
};

// real uniform buffer uploader
class VulkanUniformBufferUploader
{
public:
	VulkanUniformBufferUploader(VulkanDevice* device);

	virtual ~VulkanUniformBufferUploader();

	uint8* GetCPUMappedPointer()
	{
		return (uint8*)m_RingBuffer->GetMappedPointer();
	}

	uint64 AllocateMemory(uint64 size, uint32 alignment)
	{
		return m_RingBuffer->AllocateMemory(size, alignment);
	}

	VulkanSubBufferAllocator* GetBufferAllocator() const
	{
		return m_RingBuffer->GetBufferAllocator();
	}

	VkBuffer GetBufferHandle() const
	{
		return m_RingBuffer->GetHandle();
	}

	inline uint32 GetBufferOffset() const
	{
		return m_RingBuffer->GetBufferOffset();
	}

protected:
	VulkanDevice*			m_VulkanDevice;
	VulkanRingBuffer*		m_RingBuffer;
};

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

    inline bool GetBindingDescription(VertexAttribute attribute, BindingDescription& outBinding) const
    {
        AttributeDescription attributeDescription;
        if (!GetAttributeDescription(attribute, attributeDescription)) {
            return false;
        }
        for (int32 i = 0; i < m_Bindings.size(); ++i)
        {
            if (m_Bindings[i].binding == attributeDescription.binding)
            {
                outBinding = m_Bindings[i];
                return true;
            }
        }
        return false;
    }

    inline bool GetAttributeDescription(VertexAttribute attribute, AttributeDescription& outDescription) const
    {
        for (int32 i = 0; i < m_InputAttributes.size(); ++i)
        {
            if (m_InputAttributes[i].attribute == attribute) {
                outDescription = m_InputAttributes[i];
                return true;
            }
        }
        return false;
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