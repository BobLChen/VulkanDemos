#include "DVKMaterial.h"
#include "DVKDefaultRes.h"

namespace vk_demo
{

	DVKRingBuffer*	DVKMaterial::ringBuffer = nullptr;
	int32			DVKMaterial::ringBufferRefCount = 0;
    
	void DVKMaterial::InitRingBuffer(std::shared_ptr<VulkanDevice> vulkanDevice)
	{
		ringBuffer = new DVKRingBuffer();
		ringBuffer->device		 = vulkanDevice->GetInstanceHandle();
		ringBuffer->bufferSize   = 32 * 1024 * 1024; // 32MB
		ringBuffer->bufferOffset = ringBuffer->bufferSize;
		ringBuffer->minAlignment = vulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
		ringBuffer->realBuffer   = vk_demo::DVKBuffer::CreateBuffer(
			vulkanDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			ringBuffer->bufferSize
		);
		ringBuffer->realBuffer->Map();

		ringBufferRefCount = 0;
	}

	void DVKMaterial::DestroyRingBuffer()
	{
		delete ringBuffer;
		ringBuffer = nullptr;
		ringBufferRefCount = 0;
	}

	DVKMaterial::~DVKMaterial()
	{
		shader = nullptr;

		delete descriptorSet;
		descriptorSet = nullptr;

		textures.clear();
		uniformBuffers.clear();

		vulkanDevice = nullptr;
        
        if (pipeline) 
		{
            delete pipeline;
            pipeline = nullptr;
        }

		ringBufferRefCount -= 1;
		if (ringBufferRefCount == 0) {
			DestroyRingBuffer();
		}
	}

	DVKMaterial* DVKMaterial::Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKRenderTarget* renderTarget, VkPipelineCache pipelineCache, DVKShader* shader)
	{
		// 初始化全局RingBuffer
		if (ringBufferRefCount == 0) {
			InitRingBuffer(vulkanDevice);
		}
		ringBufferRefCount += 1;

		// 创建材质
		DVKMaterial* material = new DVKMaterial();
		material->pipelineInfo.colorAttachmentCount = renderTarget->renderPassInfo.numColorRenderTargets;

		material->vulkanDevice  = vulkanDevice;
		material->shader        = shader;
		material->renderPass    = renderTarget->GetRenderPass();
		material->pipelineCache = pipelineCache;
		material->Prepare();

		return material;
	}

	DVKMaterial* DVKMaterial::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkRenderPass renderPass, VkPipelineCache pipelineCache, DVKShader* shader)
	{
		// 初始化全局RingBuffer
		if (ringBufferRefCount == 0) {
			InitRingBuffer(vulkanDevice);
		}
		ringBufferRefCount += 1;

		// 创建材质
		DVKMaterial* material   = new DVKMaterial();
		material->vulkanDevice  = vulkanDevice;
		material->shader        = shader;
        material->renderPass    = renderPass;
        material->pipelineCache = pipelineCache;
		material->Prepare();
        
		return material;
	}

	void DVKMaterial::Prepare()
	{
        // 创建descriptorSet
        descriptorSet = shader->AllocateDescriptorSet();
        
		// 从Shader获取buffer信息
        for (auto it = shader->bufferParams.begin(); it != shader->bufferParams.end(); ++it)
        {
			DVKSimulateBuffer uboBuffer = {};
			uboBuffer.binding        = it->second.binding;
			uboBuffer.descriptorType = it->second.descriptorType;
			uboBuffer.set            = it->second.set;
			uboBuffer.stageFlags     = it->second.stageFlags;
			uboBuffer.dataSize       = it->second.bufferSize;
			uboBuffer.bufferInfo     = {};
			uboBuffer.bufferInfo.buffer = ringBuffer->realBuffer->buffer;
			uboBuffer.bufferInfo.offset = 0;
			uboBuffer.bufferInfo.range  = uboBuffer.dataSize;

			if (it->second.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
				it->second.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
			{
				// WriteBuffer，从今以后所有的UniformBuffer改为Dynamic的方式
				uniformBuffers.insert(std::make_pair(it->first, uboBuffer));
				descriptorSet->WriteBuffer(it->first, &(uboBuffer.bufferInfo));
			}
			else if (it->second.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
					 it->second.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
			{
				storageBuffers.insert(std::make_pair(it->first, uboBuffer));
			}
        }
        
        // 设置Offset的索引,DynamicOffset的顺序跟set和binding顺序相关
		dynamicOffsetCount = 0;
        std::vector<DVKDescriptorSetLayoutInfo>& setLayouts = shader->setLayoutsInfo.setLayouts;
        for (int32 i = 0; i < setLayouts.size(); ++i)
        {
            std::vector<VkDescriptorSetLayoutBinding>& bindings = setLayouts[i].bindings;
            for (int32 j = 0; j < bindings.size(); ++j)
            {
                for (auto it = uniformBuffers.begin(); it != uniformBuffers.end(); ++it)
                {
                    if (it->second.set            == setLayouts[i].set &&
						it->second.binding        == bindings[j].binding &&
                        it->second.descriptorType == bindings[j].descriptorType &&
                        it->second.stageFlags     == bindings[j].stageFlags
                    )
                    {
						it->second.dynamicIndex = dynamicOffsetCount;
						dynamicOffsetCount     += 1;
                        break;
                    }
                }
            }
        }
		globalOffsets.resize(dynamicOffsetCount);
        
		// 从Shader中获取Texture信息，包含attachment信息
        for (auto it = shader->imageParams.begin(); it != shader->imageParams.end(); ++it)
        {
            DVKSimulateTexture texture = {};
            texture.texture         = nullptr;
            texture.binding         = it->second.binding;
            texture.descriptorType  = it->second.descriptorType;
            texture.set             = it->second.set;
            texture.stageFlags      = it->second.stageFlags;
            textures.insert(std::make_pair(it->first, texture));
        }
	}
    
    void DVKMaterial::PreparePipeline()
    {
        if (pipeline) 
		{
            delete pipeline;
            pipeline = nullptr;
        }
        
		// pipeline
        pipelineInfo.shader = shader;
        pipeline = DVKGfxPipeline::Create(
            vulkanDevice,
            pipelineCache,
            pipelineInfo,
            shader->inputBindings,
			shader->inputAttributes,
            shader->pipelineLayout,
            renderPass
        );
    }

	void DVKMaterial::BeginFrame()
	{
		if (actived) {
			return;
		}
		actived = true;
		perObjectIndexes.clear();

		// 重置GlobalOffsets数据
		memset(globalOffsets.data(), MAX_uint32, sizeof(uint32) * globalOffsets.size());

		// 拷贝UniformBuffer
		for (auto it = uniformBuffers.begin(); it != uniformBuffers.end(); ++it)
		{
			if (!it->second.global) {
				continue;
			}
			// 拷贝数据至ringbuffer
			uint8* ringCPUData = (uint8*)(ringBuffer->GetMappedPointer());
			uint64 ringOffset  = ringBuffer->AllocateMemory(it->second.dataSize);
			uint64 bufferSize  = it->second.dataSize;
			// 拷贝数据
			memcpy(ringCPUData + ringOffset, it->second.dataContent.data(), bufferSize);
			// 记录Offset
			globalOffsets[it->second.dynamicIndex] = ringOffset;
		}
	}

	void DVKMaterial::EndFrame()
	{
		actived = false;
	}
    
	void DVKMaterial::BeginObject()
	{
		int32 index = perObjectIndexes.size();
		perObjectIndexes.push_back(index);

		int32 offsetStart = index * dynamicOffsetCount;
		
		// 扩充dynamicOffsets尺寸以便能够保持每个Object的参数
		if (offsetStart + dynamicOffsetCount > dynamicOffsets.size()) {
			for (int32 i = 0; i < dynamicOffsetCount; ++i) {
				dynamicOffsets.push_back(0);
			}
		}
		
		// 拷贝GlobalOffsets
		for (int32 offsetIndex = offsetStart; offsetIndex < dynamicOffsetCount; ++offsetIndex) {
			dynamicOffsets[offsetIndex] = globalOffsets[offsetIndex - offsetStart];
		}
	}

	void DVKMaterial::EndObject()
	{
		// 检查是否所有的Uniform数据都设置完成
		for (int32 i = 0; i < perObjectIndexes.size(); ++i) 
		{
			int32 offsetStart = i * dynamicOffsetCount;
			for (int32 offsetIndex = offsetStart; offsetIndex < dynamicOffsetCount; ++offsetIndex) {
				if (dynamicOffsets[offsetIndex] == MAX_uint32) {
					MLOGE("Uniform not set\n");
				}
			}
		}

		if (perObjectIndexes.size() == 0)
		{
			for (int32 i = 0; i < dynamicOffsetCount; ++i) {
				if (globalOffsets[i] == MAX_uint32) {
					MLOGE("Uniform not set\n");
				}
			}
		}
	}

	void DVKMaterial::BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, int32 objIndex)
	{
		uint32* dynOffsets = nullptr;
		if (objIndex < perObjectIndexes.size())
		{
			dynOffsets  = dynamicOffsets.data() + perObjectIndexes[objIndex] * dynamicOffsetCount;;;
		}
		else if (globalOffsets.size() > 0) 
		{
			dynOffsets  = globalOffsets.data();
		}
		
		vkCmdBindDescriptorSets(
			commandBuffer, 
			bindPoint, 
			GetPipelineLayout(), 
			0, GetDescriptorSets().size(), GetDescriptorSets().data(), 
			dynamicOffsetCount, dynOffsets
		);
	}

    void DVKMaterial::SetLocalUniform(const std::string& name, void* dataPtr, uint32 size)
    {
        auto it = uniformBuffers.find(name);
        if (it == uniformBuffers.end()) 
		{
            MLOGE("Uniform %s not found.", name.c_str());
            return;
        }
        
        if (it->second.dataSize != size) 
		{
            MLOGE("Uniform %s size not match, dst=%ud src=%ud", name.c_str(), it->second.dataSize, size);
            return;
        }

		// 获取Object的起始位置以及DynamicOffset的起始位置
		int32 objIndex     = perObjectIndexes.back();
		int32 offsetStart  = objIndex * dynamicOffsetCount;
		uint32* dynOffsets = dynamicOffsets.data() + offsetStart;

		// 拷贝数据至ringbuffer
		uint8* ringCPUData = (uint8*)(ringBuffer->GetMappedPointer());
		uint64 ringOffset  = ringBuffer->AllocateMemory(it->second.dataSize);
		uint64 bufferSize  = it->second.dataSize;
		
		// 拷贝数据
		memcpy(ringCPUData + ringOffset, dataPtr, bufferSize);

		// 记录Offset
		dynOffsets[it->second.dynamicIndex] = ringOffset;
    }

	void DVKMaterial::SetGlobalUniform(const std::string& name, void* dataPtr, uint32 size)
	{
		auto it = uniformBuffers.find(name);
		if (it == uniformBuffers.end()) 
		{
			MLOGE("Uniform %s not found.", name.c_str());
			return;
		}

		if (it->second.dataSize != size) 
		{
			MLOGE("Uniform %s size not match, dst=%ud src=%ud", name.c_str(), it->second.dataSize, size);
			return;
		}
        
		if (it->second.dataContent.size() != size) {
			it->second.dataContent.resize(size);
		}

		it->second.global = true;
		memcpy(it->second.dataContent.data(), dataPtr, size);
	}
    
    void DVKMaterial::SetTexture(const std::string& name, DVKTexture* texture)
    {
        auto it = textures.find(name);
        if (it == textures.end()) 
		{
            MLOGE("Texture %s not found.", name.c_str());
            return;
        }
        
		if (texture == nullptr) 
		{
			MLOGE("Texture %s can't be null.", name.c_str());
			return;
		}

        if (it->second.texture != texture) 
		{
            it->second.texture = texture;
            descriptorSet->WriteImage(name, texture);
        }
    }
    
	void DVKMaterial::SetInputAttachment(const std::string& name, DVKTexture* texture)
	{
		SetTexture(name, texture);
	}

	void DVKMaterial::SetStorageBuffer(const std::string& name, DVKBuffer* buffer)
	{
		auto it = storageBuffers.find(name);
		if (it == storageBuffers.end()) 
		{
			MLOGE("StorageBuffer %s not found.", name.c_str());
			return;
		}

		if (buffer == nullptr) 
		{
			MLOGE("StorageBuffer %s can't be null.", name.c_str());
			return;
		}

		if (it->second.bufferInfo.buffer != buffer->buffer) 
		{
			it->second.dataSize          = buffer->size;
			it->second.bufferInfo.buffer = buffer->buffer;
			it->second.bufferInfo.offset = 0;
			it->second.bufferInfo.range  = buffer->size;
			descriptorSet->WriteBuffer(name, buffer);
		}
	}

};
