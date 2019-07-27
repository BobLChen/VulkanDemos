#include "DVKMaterial.h"

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

	DVKMaterial* DVKMaterial::Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKShader* shader)
	{
		// 初始化全局RingBuffer
		if (ringBufferRefCount == 0) {
			InitRingBuffer(vulkanDevice);
		}
		ringBufferRefCount += 1;

		// 创建材质
		DVKMaterial* material  = new DVKMaterial();
		material->vulkanDevice = vulkanDevice;
		material->shader       = shader;
		material->Prepare();

		return material;
	}

	void DVKMaterial::Prepare()
	{
		// 从Shader获取UniformBuffer信息

		// 从Shader中获取Texture信息

		// 创建descriptorSet
	}

};