#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/RHIDefinitions.h"
#include "Vulkan/VulkanDevice.h"

#include <vector>

enum class VertexAttribute
{
	None = 0,
	Position,
	UV0,
	UV1,
	Normal,
	Tangent,
	Color,
	SkinWeight,
	SkinIndex,
	Custom0,
	Custom1,
	Custom2,
	Custom3,
	AttributeCount,
};

FORCEINLINE VertexAttribute StringToVertexAttribute(const char* name)
{
    if (strcmp(name, "inPosition") == 0)
    {
        return VertexAttribute::Position;
    }
    else if (strcmp(name, "inUV0") == 0)
    {
        return VertexAttribute::UV0;
    }
    else if (strcmp(name, "inUV1") == 0)
    {
        return VertexAttribute::UV1;
    }
    else if (strcmp(name, "inNormal") == 0)
    {
        return VertexAttribute::Normal;
    }
    else if (strcmp(name, "inTangent") == 0)
    {
        return VertexAttribute::Tangent;
    }
    else if (strcmp(name, "inColor") == 0)
    {
        return VertexAttribute::Color;
    }
    else if (strcmp(name, "inSkinWeight") == 0)
    {
        return VertexAttribute::SkinWeight;
    }
    else if (strcmp(name, "inSkinIndex") == 0)
    {
        return VertexAttribute::SkinIndex;
    }
    else if (strcmp(name, "inCustom0") == 0)
    {
        return VertexAttribute::Custom0;
    }
    else if (strcmp(name, "inCustom1") == 0)
    {
        return VertexAttribute::Custom1;
    }
    else if (strcmp(name, "inCustom2") == 0)
    {
        return VertexAttribute::Custom2;
    }
    else if (strcmp(name, "inCustom3") == 0)
    {
        return VertexAttribute::Custom3;
    }
    
    return VertexAttribute::None;
}

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
	uint8 stream;
	uint8 offset;
	VertexElementType format;
	VertexAttribute attribute;

	VertexChannelInfo()
		: stream(0)
		, offset(0)
		, format(VertexElementType::VET_None)
		, attribute(VertexAttribute::None)
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

class VertexBuffer
{
public:
	
	VertexBuffer();

	virtual ~VertexBuffer();
    
	void AddStream(const VertexStreamInfo& streamInfo, const std::vector<VertexChannelInfo>& channels, uint8* dataPtr);

	void Upload(std::shared_ptr<VulkanRHI> vulkanRHI);

	void Download(std::shared_ptr<VulkanRHI> vulkanRHI);

	FORCEINLINE int32 GetStreamCount() const
    {
        return m_Streams.size();
    }

	FORCEINLINE int32 GetStreamIndex(VertexAttribute attribute) const
    {
        uint32 channelMask = 1 << (int32)attribute;
        for (int32 i = 0; i < m_Streams.size(); ++i)
        {
            if (m_Streams[i].channelMask & channelMask)
            {
                return i;
            }
        }
        return -1;
    }

	FORCEINLINE uint32 GetChannelOffset(VertexAttribute attribute) const
	{
		return m_Channels[(int32)attribute].offset;
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

	FORCEINLINE const VertexChannelInfo& GetChannel(int32 index) const
	{
		return m_Channels[index];
	}

	FORCEINLINE const std::vector<VertexChannelInfo>& GetChannels() const
	{
		return m_Channels;
	}

	FORCEINLINE const VertexStreamInfo& GetStream(int32 index) const
	{
		return m_Streams[index];
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
		m_Valid = false;
		DestroyBuffer();
	}

	FORCEINLINE bool Valid()
	{
		if (!m_Valid)
		{
			CreateBuffer();
		}
		return m_Valid;
	}
	
    const VkPipelineVertexInputStateCreateInfo& GetVertexInputStateInfo();

protected:
	void DestroyBuffer();

	void CreateBuffer();

protected:
	std::vector<VertexChannelInfo> m_Channels;
	std::vector<VertexStreamInfo>  m_Streams;
	std::vector<uint8*>            m_Datas;

	std::vector<VkDeviceMemory>    m_Memories;
	std::vector<VkBuffer>		   m_Buffers;

	uint32 m_VertexCount;
	uint32 m_DataSize;
	uint32 m_CurrentChannels;
	bool m_Valid;
    
    bool m_InputStateDirty;
    VkPipelineVertexInputStateCreateInfo m_VertexInputStateInfo;
    std::vector<VkVertexInputBindingDescription> m_VertexBindings;
    std::vector<VkVertexInputAttributeDescription> m_VertexInputAttris;
};
