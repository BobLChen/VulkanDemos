#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "VulkanPlatform.h"
#include "RHIDefinitions.h"
#include "VulkanDevice.h"

#include <vector>

class IndexBuffer
{
public:
	
	IndexBuffer(uint8* dataPtr, uint32 dataSize, PrimitiveType primitiveType, VkIndexType indexType);

	virtual ~IndexBuffer();

	void Upload(std::shared_ptr<VulkanRHI> vulkanRHI);

	void Download(std::shared_ptr<VulkanRHI> vulkanRHI);

	FORCEINLINE VkIndexType GetIndexType() const
	{
		return m_IndexType;
	}

	FORCEINLINE PrimitiveType GetPrimitiveType() const
	{
		return m_PrimitiveType;
	}

	FORCEINLINE const uint8* GetData() const
	{
		return m_Data;
	}

	FORCEINLINE uint32 GetDataSize() const
	{
		return m_DataSize;
	}

	FORCEINLINE uint32 GetIndexCount() const
	{
		return m_IndexCount;
	}

	FORCEINLINE VkBuffer GetBuffer() const
	{
		return m_Buffer;
	}

	FORCEINLINE VkDeviceMemory GetMemory() const
	{
		return m_Memory;
	}

	FORCEINLINE uint32 GetAlignment() const
	{
		return m_Alignment;
	}

	FORCEINLINE uint32 GetAllocationSize() const
	{
		return m_AllocationSize;
	}

	FORCEINLINE uint32 GetTriangleCount() const
	{
		return m_TriangleCount;
	}

protected:

	VkIndexType m_IndexType;
	PrimitiveType m_PrimitiveType;

	uint8* m_Data;
	uint32 m_DataSize;
	uint32 m_IndexCount;
	uint32 m_TriangleCount;
	VkBuffer m_Buffer;
	VkDeviceMemory m_Memory;

	bool m_Uploaded;

	uint32 m_AllocationSize;
	uint32 m_Alignment;
};