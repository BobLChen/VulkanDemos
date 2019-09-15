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
#include "DVKRenderTarget.h"

#include "Math/Math.h"
#include "Utils/Alignment.h"
#include "Vulkan/VulkanCommon.h"

namespace vk_demo
{

	struct DVKSimulateBuffer
	{
		std::vector<uint8>		dataContent;
		bool                    global = false;
		uint32					dataSize = 0;
        uint32					set = 0;
        uint32					binding = 0;
        uint32					dynamicIndex = 0;
        VkDescriptorType		descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        VkShaderStageFlags		stageFlags = 0;
		VkDescriptorBufferInfo	bufferInfo;
	};
    
    struct DVKSimulateTexture
    {
        uint32              set = 0;
        uint32              binding = 0;
        VkDescriptorType    descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
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
            
			if (allocationOffset + size <= bufferSize) 
			{
				bufferOffset = allocationOffset + size;
				return allocationOffset;
			}

			bufferOffset = 0;
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

		typedef std::unordered_map<std::string, DVKSimulateBuffer>		BuffersMap;
		typedef std::unordered_map<std::string, DVKSimulateTexture>		TexturesMap;
		typedef std::shared_ptr<VulkanDevice>							VulkanDeviceRef;

		DVKMaterial()
		{
            
		}
        
	public:
		virtual ~DVKMaterial();

		static DVKMaterial* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkRenderPass renderPass, VkPipelineCache pipelineCache, DVKShader* shader);
        
		static DVKMaterial* Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKRenderTarget* renderTarget, VkPipelineCache pipelineCache, DVKShader* shader);

        void PreparePipeline();

		void BeginObject();

		void EndObject();

		void BeginFrame();

		void EndFrame();

		void BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, int32 objIndex);

        void SetLocalUniform(const std::string& name, void* dataPtr, uint32 size);
        
        void SetTexture(const std::string& name, DVKTexture* texture);

		void SetGlobalUniform(const std::string& name, void* dataPtr, uint32 size);

		void SetStorageBuffer(const std::string& name, DVKBuffer* buffer);

		void SetInputAttachment(const std::string& name, DVKTexture* texture);

		inline VkPipeline GetPipeline() const
		{
			return pipeline->pipeline;
		}

		inline VkPipelineLayout GetPipelineLayout() const
		{
			return pipeline->pipelineLayout;
		}

		inline std::vector<VkDescriptorSet>& GetDescriptorSets() const
		{
			return descriptorSet->descriptorSets;
		}
        
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
        
        VkRenderPass            renderPass = VK_NULL_HANDLE;
        VkPipelineCache         pipelineCache = VK_NULL_HANDLE;
        
        DVKGfxPipelineInfo      pipelineInfo;
        DVKGfxPipeline*         pipeline = nullptr;
        DVKDescriptorSet*		descriptorSet = nullptr;

		uint32					dynamicOffsetCount;
		std::vector<uint32>		globalOffsets;
        std::vector<uint32>     dynamicOffsets;
		std::vector<uint32>		perObjectIndexes;
        
		BuffersMap				uniformBuffers;
		BuffersMap				storageBuffers;
		TexturesMap				textures;

		bool                    actived = false;
	};

}
