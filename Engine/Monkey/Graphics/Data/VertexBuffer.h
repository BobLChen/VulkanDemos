#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/RHIDefinitions.h"
#include <vector>

class VertexBuffer
{
public:
	
	VertexBuffer();

	virtual ~VertexBuffer();
    
	void AddStream(const VertexStreamInfo& streamInfo, const std::vector<VertexChannelInfo>& channels, uint8* dataPtr);

	FORCEINLINE int32 GetStreamCount() const
    {
        return int32(m_Streams.size());
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

    FORCEINLINE void UpdateResources()
    {
        if (m_Invalid)
        {
            CreateBuffer();
        }
    }
	
    const VertexInputDeclareInfo& GetVertexInputStateInfo() const
	{
		return m_VertexInputStateInfo;
	}

protected:

	void DestroyBuffer();

	void CreateBuffer();

	void UpdateVertexInputState();

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
	VertexInputDeclareInfo			m_VertexInputStateInfo;

	uint32							m_Hash;
};
