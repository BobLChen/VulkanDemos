#pragma once

#include <string>
#include <cstring>
#include <memory>
#include <unordered_map>

#include "DVKUtils.h"
#include "DVKBuffer.h"
#include "DVKTexture.h"
#include "DVKShader.h"

#include "Math/Math.h"
#include "File/FileManager.h"
#include "Vulkan/VulkanCommon.h"

namespace vk_demo
{

	class DVKSimulateUniformBuffer
	{
	public:
		uint8*		dataPtr;
		uint32		dataSize;
	};

	class DVKRingBuffer
	{
	public:
		DVKRingBuffer()
		{

		}

		virtual ~DVKRingBuffer()
		{
			realBuffer->UnMap();
			delete realBuffer;
			realBuffer = nullptr;
		}

		void* GetMappedPointer()
		{
			return realBuffer->mapped;
		}

		uint64 AllocateMemory(uint64 size, uint32 alignment)
		{
			alignment = MMath::Max(alignment, minAlignment);
			uint64 allocationOffset = Align<uint64>(bufferOffset, alignment);

			if (allocationOffset + size <= bufferSize) {
				bufferOffset = allocationOffset + size;
				return allocationOffset;
			}

			bufferOffset = Align<uint64>(size, alignment);;
			return bufferOffset;
		}

	public:

		VkDevice		device = VK_NULL_HANDLE;
		uint64			bufferSize = 0;
		uint64			bufferOffset = 0;
		uint32			minAlignment = 0;
		DVKBuffer*		realBuffer = nullptr;

	};

	class DVKMaterial
	{
	private:

		typedef std::unordered_map<std::string, DVKSimulateUniformBuffer>	UniformBuffersMap;
		typedef std::unordered_map<std::string, DVKTexture*>				TexturesMap;
		typedef std::shared_ptr<VulkanDevice>								VulkanDeviceRef;

		DVKMaterial()
		{

		}

	public:
		virtual ~DVKMaterial();

		DVKMaterial* Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKShader* shader);

	private:
		static void InitRingBuffer(std::shared_ptr<VulkanDevice> vulkanDevice);

		static void DestroyRingBuffer();

		void Prepare();

	private:

		static DVKRingBuffer*	ringBuffer;
		static int32			ringBufferRefCount;

	public:

		VulkanDeviceRef			vulkanDevice = nullptr;
		DVKShader*				shader = nullptr;
		DVKDescriptorSet*		descriptorSet = nullptr;
		UniformBuffersMap		uniformBuffers;
		TexturesMap				textures;

	};

}