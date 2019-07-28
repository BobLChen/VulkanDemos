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

		ringBufferRefCount -= 1;
		if (ringBufferRefCount == 0) {
			DestroyRingBuffer();
		}
	}

	DVKMaterial* DVKMaterial::Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkRenderPass renderPass, VkPipelineCache pipelineCache, DVKShader* shader, DVKModel* model)
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
        material->model         = model;
        material->renderPass    = renderPass;
        material->pipelineCache = pipelineCache;
		material->Prepare();
        
		return material;
	}

	void DVKMaterial::Prepare()
	{
        // 创建descriptorSet
        descriptorSet = shader->AllocateDescriptorSet();
        
		// 从Shader获取UniformBuffer信息
        for (auto it = shader->uboParams.begin(); it != shader->uboParams.end(); ++it)
        {
            DVKSimulateUniformBuffer uboBuffer = {};
            uboBuffer.dataPtr        = nullptr;
            uboBuffer.binding        = it->second.binding;
            uboBuffer.dataSize       = it->second.bufferSize;
            uboBuffer.descriptorType = it->second.descriptorType;
            uboBuffer.set            = it->second.set;
            uboBuffer.stageFlags     = it->second.stageFlags;
            uniformBuffers.insert(std::make_pair(it->first, uboBuffer));
            // 设置默认值，从今以后所有的UniformBuffer改为Dynamic的方式
            descriptorSet->WriteBuffer(it->first, ringBuffer->realBuffer);
        }
        
        // 设置Offset的索引,DynamicOffset的顺序跟set和binding顺序相关
        std::vector<DVKDescriptorSetLayoutInfo>& setLayouts = shader->setLayoutsInfo.setLayouts;
        for (int32 i = 0; i < setLayouts.size(); ++i)
        {
            std::vector<VkDescriptorSetLayoutBinding>& bindings = setLayouts[i].bindings;
            for (int32 j = 0; j < bindings.size(); ++j)
            {
                dynamicOffsets.push_back(0);
                for (auto it = uniformBuffers.begin(); it != uniformBuffers.end(); ++it)
                {
                    if (it->second.binding        == bindings[j].binding &&
                        it->second.set            == i && setLayouts[i].set &&
                        it->second.descriptorType == bindings[j].descriptorType &&
                        it->second.stageFlags     == bindings[j].stageFlags
                    )
                    {
                        it->second.dynamicIndex = dynamicOffsets.size() - 1;
                        break;
                    }
                }
            }
        }
        
		// 从Shader中获取Texture信息
        for (auto it = shader->texParams.begin(); it != shader->texParams.end(); ++it)
        {
            DVKSimulateTexture texture = {};
            texture.texture         = nullptr;
            texture.binding         = it->second.binding;
            texture.descriptorType  = it->second.descriptorType;
            texture.set             = it->second.set;
            texture.stageFlags      = it->second.stageFlags;
            textures.insert(std::make_pair(it->first, texture));
            // 设置默认贴图
            descriptorSet->WriteImage(it->first, DVKDefaultRes::texture2D);
        }
	}
    
    void DVKMaterial::PreparePipeline()
    {
        if (pipeline) {
            delete pipeline;
            pipeline = nullptr;
        }
        
        pipelineInfo.shader = shader;
        pipeline = DVKPipeline::Create(
            vulkanDevice,
            pipelineCache,
            pipelineInfo,
            {
               model->GetInputBinding()
            },
            model->GetInputAttributes(),
            shader->pipelineLayout,
            renderPass
        );
    }
    
    void DVKMaterial::SetUniform(const std::string& name, void* dataPtr, uint32 size)
    {
        auto it = uniformBuffers.find(name);
        if (it == uniformBuffers.end()) {
            MLOGE("Uniform %s not found.", name.c_str());
            return;
        }
        
        if (it->second.dataSize != size) {
            MLOGE("Uniform %s size not match, dst=%ud src=%ud", name.c_str(), it->second.dataSize, size);
            return;
        }
        
        if (it->second.dataPtr != dataPtr) {
            it->second.dataPtr = dataPtr;
        }
    }
    
    void DVKMaterial::SetTexture(const std::string& name, DVKTexture* texture)
    {
        auto it = textures.find(name);
        if (it == textures.end()) {
            MLOGE("Texture %s not found.", name.c_str());
            return;
        }
        
        if (it->second.texture != texture) {
            it->second.texture = texture;
            descriptorSet->WriteImage(name, texture);
        }
    }
    
    void DVKMaterial::Update()
    {
        // 拷贝数据到ringbuffer
        for (auto it = uniformBuffers.begin(); it != uniformBuffers.end(); ++it)
        {
            dynamicOffsets[it->second.dynamicIndex] = ringBuffer->AllocateMemory(it->second.dataSize);
            if (it->second.dataPtr) {
                memcpy(ringBuffer->GetMappedPointer(), it->second.dataPtr, it->second.dataSize);
            }
        }
    }
    
};
