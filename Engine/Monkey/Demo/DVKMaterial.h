#pragma once

#include <string>
#include <cstring>
#include <memory>
#include <unordered_map>

#include "DVKUtils.h"
#include "DVKBuffer.h"
#include "DVKTexture.h"
#include "DVKShader.h"
#include "DVKPipeline.h"
#include "DVKModel.h"

#include "Math/Math.h"
#include "File/FileManager.h"
#include "Vulkan/VulkanCommon.h"

namespace vk_demo
{

	struct DVKSimulateUniformBuffer
	{
		void*		        dataPtr;
		uint32		        dataSize;
        uint32              set = 0;
        uint32              binding = 0;
        uint32              dynamicIndex = 0;
        VkDescriptorType    descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        VkShaderStageFlags  stageFlags = 0;
	};
    
    struct DVKSimulateTexture
    {
        uint32              set = 0;
        uint32              binding = 0;
        VkDescriptorType    descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        VkShaderStageFlags  stageFlags = 0;
        DVKTexture*         texture = nullptr;
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

		uint64 AllocateMemory(uint64 size)
		{
			uint64 allocationOffset = Align<uint64>(bufferOffset, minAlignment);
            
			if (allocationOffset + size <= bufferSize) {
				bufferOffset = allocationOffset + size;
				return allocationOffset;
			}

			bufferOffset = Align<uint64>(size, minAlignment);
            allocationOffset = 0;
            
			return allocationOffset;
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
		typedef std::unordered_map<std::string, DVKSimulateTexture>			TexturesMap;
		typedef std::shared_ptr<VulkanDevice>								VulkanDeviceRef;

		DVKMaterial()
		{
            
		}
        
	public:
		virtual ~DVKMaterial();

		static DVKMaterial* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkRenderPass renderPass, VkPipelineCache pipelineCache, DVKShader* shader, DVKModel* model);
        
        void PreparePipeline();
        
        void Update();
        
        void SetUniform(const std::string& name, void* dataPtr, uint32 size);
        
        void SetTexture(const std::string& name, DVKTexture* texture);
        
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
        DVKModel*               model = nullptr;
        
        VkRenderPass            renderPass = VK_NULL_HANDLE;
        VkPipelineCache         pipelineCache = VK_NULL_HANDLE;
        
        DVKPipelineInfo         pipelineInfo;
        DVKPipeline*            pipeline;
        DVKDescriptorSet*		descriptorSet = nullptr;
        std::vector<uint32>     dynamicOffsets;
        
		UniformBuffersMap		uniformBuffers;
		TexturesMap				textures;
	};

}
