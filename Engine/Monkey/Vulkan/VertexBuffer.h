#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "VulkanPlatform.h"
#include "RHIDefinitions.h"

#include <vector>

enum class VertexAttribute
{
	Position = 1,
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

struct VertexStreamInfo
{
	uint32 channelMask;
	uint32 offset;
	uint8  stride;
	
	VertexStreamInfo()
		: channelMask(0)
		, offset(0)
		, stride(0)
	{

	}

	bool operator == (const VertexStreamInfo& rhs) const
	{
		return channelMask == rhs.channelMask && offset == rhs.offset && stride == rhs.stride;
	}

	bool operator != (const VertexStreamInfo& rhs) const
	{
		return !(*this == rhs);
	}
};

struct VertexChannelInfo
{
	uint8 stream;
	uint8 offset;
	uint8 format;
	uint8 dimension;

	VertexChannelInfo()
		: stream(0)
		, offset(0)
		, format(0)
		, dimension(0)
	{

	}

	uint32 CalcOffset(const std::vector<VertexStreamInfo>& streams) const
	{
		return streams[stream].offset + offset;
	}

	uint32 CalcStride(const std::vector<VertexStreamInfo>& streams) const
	{
		return streams[stream].stride;
	}
	
	bool IsValid() const
	{
		return dimension != 0;
	}

	void Reset()
	{
		*this = VertexChannelInfo();
	}

	bool operator == (const VertexChannelInfo& rhs) const
	{
		return stream == rhs.stream && offset == rhs.offset && format == rhs.format && dimension == rhs.dimension;
	}

	bool operator != (const VertexChannelInfo& rhs) const
	{
		return !(*this == rhs);
	}
};

class VertexData
{
public:
	
	VertexData();

	virtual ~VertexData();

	int32 GetStreamCount() const;

	int32 GetStreamIndex(VertexAttribute attribute) const;

	uint32 GetChannelOffset(VertexAttribute attribute) const
	{ 
		return m_Channels[(int32)attribute].CalcOffset(m_Streams);
	}

	uint32 GetChannelStride(VertexAttribute attribute) const
	{ 
		return m_Channels[(int32)attribute].CalcStride(m_Streams);
	}
	
	uint8* GetDataPtr() const 
	{ 
		return m_Data;
	}

	int32 GetDataSize() const 
	{ 
		return m_DataSize;
	}

	int32 GetVertexSize() const 
	{ 
		return m_VertexSize;
	}

	int32 GetVertexCount() const 
	{ 
		return m_VertexCount;
	}

	uint32 GetChannelMask() const
	{
		return m_CurrentChannels;
	}

	const std::vector<VertexStreamInfo>& GetStreams() const
	{
		return m_Streams;
	}

	const VertexChannelInfo& GetChannel(int32 index) const
	{
		return m_Channels[index];
	}

	const std::vector<VertexChannelInfo>& GetChannels() const
	{
		return m_Channels;
	}

	const VertexStreamInfo& GetStream(int32 index) const
	{
		return m_Streams[index];
	}

	bool HasAttribute(VertexAttribute attribute) const
	{
		return m_CurrentChannels & (1 << (int32)attribute);
	}

protected:
	std::vector<VertexChannelInfo> m_Channels;
	std::vector<VertexStreamInfo>  m_Streams;

	uint32 m_VertexSize;
	uint32 m_VertexCount;

	uint8* m_Data;
	uint32 m_DataSize;

	uint32 m_CurrentChannels;
};