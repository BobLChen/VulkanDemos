#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/RHIDefinitions.h"

#include <vector>

class IndexBuffer
{
public:
	
	IndexBuffer(uint8* dataPtr, uint32 dataSize, PrimitiveType primitiveType, VkIndexType indexType);

	virtual ~IndexBuffer();

	inline VkIndexType GetIndexType() const
	{
		return m_IndexType;
	}

	inline PrimitiveType GetPrimitiveType() const
	{
		return m_PrimitiveType;
	}

	inline const uint8* GetData() const
	{
		return m_Data;
	}

	inline uint32 GetDataSize() const
	{
		return m_DataSize;
	}

	inline uint32 GetIndexCount() const
	{
		return m_IndexCount;
	}

	inline VkBuffer GetBuffer() const
	{
		return m_Buffer;
	}

	inline VkDeviceMemory GetMemory() const
	{
		return m_Memory;
	}

	inline uint32 GetAlignment() const
	{
		return m_Alignment;
	}

	inline uint32 GetAllocationSize() const
	{
		return m_AllocationSize;
	}

	inline uint32 GetTriangleCount() const
	{
		return m_TriangleCount;
	}

	inline uint32 GetHash() const
	{
		return m_Hash;
	}
    
    inline void UpdateResources()
    {
        if (m_Invalid) {
            CreateBuffer();
        }
    }
	
protected:

	void CreateBuffer();

	void DestroyBuffer();

protected:
	VkIndexType         m_IndexType;
	PrimitiveType       m_PrimitiveType;
    
	uint8*              m_Data;
	uint32              m_DataSize;
	uint32              m_IndexCount;
	uint32              m_TriangleCount;
    
	VkBuffer            m_Buffer;
	VkDeviceMemory      m_Memory;
    
	bool                m_Invalid;
	uint32              m_AllocationSize;
	uint32              m_Alignment;

	uint32				m_Hash;
};
