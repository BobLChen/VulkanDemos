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
#include "DVKMaterial.h"

#include "Math/Math.h"
#include "Vulkan/VulkanCommon.h"

namespace vk_demo
{
    class DVKCompute
    {
    private:
        
        typedef std::unordered_map<std::string, DVKSimulateBuffer>    BuffersMap;
        typedef std::unordered_map<std::string, DVKSimulateTexture>   TexturesMap;
        typedef std::shared_ptr<VulkanDevice>                         VulkanDeviceRef;
        
        DVKCompute()
        {
            
        }
        
    public:
        virtual ~DVKCompute();
        
        static DVKCompute* Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkPipelineCache pipelineCache, DVKShader* shader);
        
        void BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint);

		void BindDispatch(VkCommandBuffer commandBuffer, int groupX, int groupY, int groupZ);
        
        void SetUniform(const std::string& name, void* dataPtr, uint32 size);
        
        void SetTexture(const std::string& name, DVKTexture* texture);
        
        void SetStorageTexture(const std::string& name, DVKTexture* texture);

		void SetStorageBuffer(const std::string& name, DVKBuffer* buffer);
        
        inline VkPipeline GetPipeline() const
        {
            return pipeline;
        }
        
        inline VkPipelineLayout GetPipelineLayout() const
        {
            return shader->pipelineLayout;
        }
        
        inline std::vector<VkDescriptorSet>& GetDescriptorSets() const
        {
            return descriptorSet->descriptorSets;
        }
        
    private:
        static void InitRingBuffer(std::shared_ptr<VulkanDevice> vulkanDevice);
        
        static void DestroyRingBuffer();
        
        void Prepare();
        
        void PreparePipeline();

    private:
        
        static DVKRingBuffer*   ringBuffer;
        static int32            ringBufferRefCount;
        
    public:
        
        VulkanDeviceRef             vulkanDevice = nullptr;
        DVKShader*                  shader = nullptr;
        
        VkPipelineCache             pipelineCache = VK_NULL_HANDLE;
        VkPipeline                  pipeline = VK_NULL_HANDLE;
        
        DVKDescriptorSet*           descriptorSet = nullptr;
        
        uint32                      dynamicOffsetCount;
        std::vector<uint32>         dynamicOffsets;
        
		BuffersMap					uniformBuffers;
		BuffersMap					storageBuffers;
        TexturesMap                 textures;
    };
    
}
