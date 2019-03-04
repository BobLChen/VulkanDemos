#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/RHIDefinitions.h"
#include "Vulkan/VulkanDevice.h"
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

	FORCEINLINE void Reset()
	{
		*this = VertexStreamInfo();
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

	FORCEINLINE void Reset()
	{
		*this = VertexChannelInfo();
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
		uint32_t			binding;
		VkFormat			format;
		uint32_t			offset;
		VertexAttribute		attribute;
	};

public:
	VertexInputDeclareInfo()
		: m_Valid(false)
		, m_hash(0)
	{

	}

	FORCEINLINE void AddBinding(const BindingDescription& binding)
	{
		m_Valid = false;
		m_Bindings.push_back(binding);
	}

	FORCEINLINE void AddAttribute(const AttributeDescription& attribute)
	{
		m_Valid = false;
		m_InputAttributes.push_back(attribute);
	}

	FORCEINLINE uint32 GetHash() const
	{
		return m_hash;
	}

	FORCEINLINE void Clear()
	{
		m_Valid = false;
		m_hash  = 0;
		m_Bindings.clear();
		m_InputAttributes.clear();
	}

	FORCEINLINE const std::vector<BindingDescription>& GetBindings() const
	{
		return m_Bindings;
	}

	FORCEINLINE const std::vector<AttributeDescription>& GetAttributes() const
	{
		return m_InputAttributes;
	}

	FORCEINLINE void Update()
	{
		if (!m_Valid)
		{
			m_hash  = 0;
			m_hash  = Crc::MemCrc32(m_Bindings.data(), m_Bindings.size() * sizeof(BindingDescription));
			m_hash  = Crc::MemCrc32(m_InputAttributes.data(), m_InputAttributes.size() * sizeof(AttributeDescription), m_hash);
			m_Valid = true;
		}
	}

protected:
	bool	m_Valid;
	uint32	m_hash;

	std::vector<BindingDescription>		m_Bindings;
	std::vector<AttributeDescription>	m_InputAttributes;
};

class VertexBuffer
{
public:
	
	VertexBuffer();

	virtual ~VertexBuffer();
    
	void AddStream(const VertexStreamInfo& streamInfo, const std::vector<VertexChannelInfo>& channels, uint8* dataPtr);

	FORCEINLINE int32 GetStreamCount() const
    {
        return m_Streams.size();
    }

	FORCEINLINE int32 GetStreamIndex(VertexAttribute attribute) const
    {
        uint32 channelMask = 1 << attribute;
        for (int32 i = 0; i < m_Streams.size(); ++i)
        {
            if (m_Streams[i].channelMask & channelMask)
            {
                return i;
            }
        }

        return -1;
    }

	FORCEINLINE int32 GetChannelIndex(VertexAttribute attribute) const
	{
		for (int32 i = 0; i < m_Channels.size(); ++i)
		{
			if (m_Channels[i].attribute == attribute)
			{
				return i;
			}
		}

		return -1;
	}

	FORCEINLINE const VertexStreamInfo& GetStream(int32 index) const
	{
		return m_Streams[index];
	}

	FORCEINLINE const VertexChannelInfo& GetChannel(int32 index) const
	{
		return m_Channels[index];
	}

	FORCEINLINE const VertexStreamInfo& GetStream(VertexAttribute attribute) const
	{
		int32 index = GetStreamIndex(attribute);
		return GetStream(index);
	}

	FORCEINLINE const VertexChannelInfo& GetChannel(VertexAttribute attribute) const
	{
		int32 index = GetChannelIndex(attribute);
		return GetChannel(index);
	}

	FORCEINLINE uint8* GetDataPtr(int32 stream) const
	{ 
		return m_Datas[stream];
	}

	FORCEINLINE int32 GetDataSize() const
	{ 
		return m_DataSize;
	}

	FORCEINLINE int32 GetVertexCount() const
	{ 
		return m_VertexCount;
	}

	FORCEINLINE uint32 GetChannelMask() const
	{
		return m_CurrentChannels;
	}

	FORCEINLINE const std::vector<VertexStreamInfo>& GetStreams() const
	{
		return m_Streams;
	}

	FORCEINLINE const std::vector<VertexChannelInfo>& GetChannels() const
	{
		return m_Channels;
	}

	FORCEINLINE bool HasAttribute(VertexAttribute attribute) const
	{
		return m_CurrentChannels & (1 << (int32)attribute);
	}

	FORCEINLINE const std::vector<VkBuffer>& GetVKBuffers() const
	{
		return m_Buffers;
	}

	FORCEINLINE const std::vector<VkDeviceMemory>& GetVKMemories() const
	{
		return m_Memories;
	}

	FORCEINLINE void Invalid()
	{
		DestroyBuffer();
        m_Invalid = true;
	}
    
	FORCEINLINE bool IsValid()
	{
		if (m_Invalid)
		{
			CreateBuffer();
		}
		return !m_Invalid;
	}
	
    const VertexInputDeclareInfo& GetVertexInputStateInfo();

protected:

	void DestroyBuffer();

	void CreateBuffer();

protected:

	std::vector<VertexChannelInfo>	m_Channels;
	std::vector<VertexStreamInfo>	m_Streams;
	std::vector<uint8*>				m_Datas;

	std::vector<VkDeviceMemory>		m_Memories;
	std::vector<VkBuffer>			m_Buffers;

	uint32							m_VertexCount;
	uint32							m_DataSize;
	uint32							m_CurrentChannels;

	bool							m_Invalid;
    bool							m_InputStateDirty;
	VertexInputDeclareInfo			m_VertexInputStateInfo;
};
