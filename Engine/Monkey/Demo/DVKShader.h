#pragma once

#include <string>
#include <cstring>
#include <memory>
#include <unordered_map>

#include "DVKUtils.h"
#include "DVKBuffer.h"
#include "DVKTexture.h"

#include "FileManager.h"
#include "Vulkan/VulkanCommon.h"

namespace spirv_cross
{
	class Compiler;
	struct ShaderResources;
}

namespace vk_demo
{
    
	class DVKDescriptorSetLayoutInfo
	{
	private:

		typedef std::vector<VkDescriptorSetLayoutBinding> BindingsArray;

	public:
		DVKDescriptorSetLayoutInfo()
		{

		}

		~DVKDescriptorSetLayoutInfo()
		{

		}

	public:
		int32			set = -1;
		BindingsArray	bindings;
	};

	class DVKDescriptorSetLayoutsInfo
	{
	public:
		struct BindInfo
		{
			int32 set;
			int32 binding;
		};

		DVKDescriptorSetLayoutsInfo()
		{

		}

		~DVKDescriptorSetLayoutsInfo()
		{

		}

		VkDescriptorType GetDescriptorType(int32 set, int32 binding)
		{
			for (int32 i = 0; i < setLayouts.size(); ++i)
			{
				if (setLayouts[i].set == set)
				{
					for (int32 j = 0; j < setLayouts[i].bindings.size(); ++j)
					{
						if (setLayouts[i].bindings[j].binding == binding)
						{
							return setLayouts[i].bindings[j].descriptorType;
						}
					}
				}
			}
			
			return VK_DESCRIPTOR_TYPE_END_RANGE;
		}

		void AddDescriptorSetLayoutBinding(const std::string& varName, int32 set, VkDescriptorSetLayoutBinding binding)
		{
			DVKDescriptorSetLayoutInfo* setLayout = nullptr;

			for (int32 i = 0; i < setLayouts.size(); ++i)
			{
				if (setLayouts[i].set == set)
				{
					setLayout = &(setLayouts[i]);
					break;
				}
			}

			if (setLayout == nullptr)
			{
				setLayouts.push_back({ });
				setLayout = &(setLayouts[setLayouts.size() - 1]);
			}

			for (int32 i = 0; i < setLayout->bindings.size(); ++i)
			{
				VkDescriptorSetLayoutBinding& setBinding = setLayout->bindings[i];
				if (setBinding.binding == binding.binding && setBinding.descriptorType == binding.descriptorType)
				{
					setBinding.stageFlags = setBinding.stageFlags | binding.stageFlags;
					return;
				}
			}
			
			setLayout->set = set;
			setLayout->bindings.push_back(binding);

			// 保存变量映射信息
			BindInfo paramInfo = {};
			paramInfo.set      = set;
			paramInfo.binding  = binding.binding;
			paramsMap.insert(std::make_pair(varName, paramInfo));
		}

	public:
		std::unordered_map<std::string, BindInfo>	paramsMap;
		std::vector<DVKDescriptorSetLayoutInfo>		setLayouts;
	};

	struct DVKAttribute
	{
		VertexAttribute	attribute;
		int32			location;
	};

	class DVKDescriptorSet
	{
	public:

		DVKDescriptorSet()
		{

		}

		~DVKDescriptorSet()
		{
			
		}
        
		void WriteImage(const std::string& name, DVKTexture* texture)
		{
			auto it = setLayoutsInfo.paramsMap.find(name);
			if (it == setLayoutsInfo.paramsMap.end()) 
			{
				MLOGE("Failed write buffer, %s not found!", name.c_str());
				return;
			}

			auto bindInfo = it->second;

			VkWriteDescriptorSet writeDescriptorSet;
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet          = descriptorSets[bindInfo.set];
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = setLayoutsInfo.GetDescriptorType(bindInfo.set, bindInfo.binding);
			writeDescriptorSet.pBufferInfo     = nullptr;
			writeDescriptorSet.pImageInfo      = &(texture->descriptorInfo);
			writeDescriptorSet.dstBinding      = bindInfo.binding;
			vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
		}

		void WriteBuffer(const std::string& name, const VkDescriptorBufferInfo* bufferInfo)
		{
			auto it = setLayoutsInfo.paramsMap.find(name);
			if (it == setLayoutsInfo.paramsMap.end()) 
			{
				MLOGE("Failed write buffer, %s not found!", name.c_str());
				return;
			}

			auto bindInfo = it->second;

			VkWriteDescriptorSet writeDescriptorSet;
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet          = descriptorSets[bindInfo.set];
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = setLayoutsInfo.GetDescriptorType(bindInfo.set, bindInfo.binding);
			writeDescriptorSet.pBufferInfo     = bufferInfo;
			writeDescriptorSet.dstBinding      = bindInfo.binding;
			vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
		}

		void WriteBuffer(const std::string& name, DVKBuffer* buffer)
		{
			auto it = setLayoutsInfo.paramsMap.find(name);
			if (it == setLayoutsInfo.paramsMap.end()) 
			{
				MLOGE("Failed write buffer, %s not found!", name.c_str());
				return;
			}

			auto bindInfo = it->second;

			VkWriteDescriptorSet writeDescriptorSet;
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet          = descriptorSets[bindInfo.set];
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = setLayoutsInfo.GetDescriptorType(bindInfo.set, bindInfo.binding);
			writeDescriptorSet.pBufferInfo     = &(buffer->descriptor);
			writeDescriptorSet.dstBinding      = bindInfo.binding;
			vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
		}

	public:

		VkDevice	device;

		DVKDescriptorSetLayoutsInfo		setLayoutsInfo;
		std::vector<VkDescriptorSet>	descriptorSets;
	};

	class DVKDescriptorSetPool
	{
	public:
		DVKDescriptorSetPool(VkDevice inDevice, int32 inMaxSet, const DVKDescriptorSetLayoutsInfo& setLayoutsInfo, const std::vector<VkDescriptorSetLayout>& inDescriptorSetLayouts)
		{
			device  = inDevice;
			maxSet  = inMaxSet;
			usedSet = 0;
			descriptorSetLayouts = inDescriptorSetLayouts;

			std::vector<VkDescriptorPoolSize> poolSizes;
			for (int32 i = 0; i < setLayoutsInfo.setLayouts.size(); ++i)
			{
				const DVKDescriptorSetLayoutInfo& setLayoutInfo = setLayoutsInfo.setLayouts[i];
				for (int32 j = 0; j < setLayoutInfo.bindings.size(); ++j)
				{
					VkDescriptorPoolSize poolSize = {};
					poolSize.type            = setLayoutInfo.bindings[j].descriptorType;
					poolSize.descriptorCount = setLayoutInfo.bindings[j].descriptorCount;
					poolSizes.push_back(poolSize);
				}
			}

			VkDescriptorPoolCreateInfo descriptorPoolInfo;
			ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
			descriptorPoolInfo.poolSizeCount = poolSizes.size();
			descriptorPoolInfo.pPoolSizes    = poolSizes.data();
			descriptorPoolInfo.maxSets       = maxSet;
			VERIFYVULKANRESULT(vkCreateDescriptorPool(inDevice, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &descriptorPool));
		}

		~DVKDescriptorSetPool()
		{
			if (descriptorPool != VK_NULL_HANDLE)
			{
				vkDestroyDescriptorPool(device, descriptorPool, VULKAN_CPU_ALLOCATOR);
				descriptorPool = VK_NULL_HANDLE;
			}
		}

		bool IsFull()
		{
			return usedSet >= maxSet;
		}

		bool AllocateDescriptorSet(VkDescriptorSet* descriptorSet)
		{
			if (usedSet + descriptorSetLayouts.size() >= maxSet) {
				return false;
			}

			usedSet += descriptorSetLayouts.size();

			VkDescriptorSetAllocateInfo allocInfo;
			ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
			allocInfo.descriptorPool     = descriptorPool;
			allocInfo.descriptorSetCount = descriptorSetLayouts.size();
			allocInfo.pSetLayouts        = descriptorSetLayouts.data();
			VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &allocInfo, descriptorSet));

			return true;
		}
		
	public:
		int32								maxSet;
		int32								usedSet;
		VkDevice							device = VK_NULL_HANDLE;
		std::vector<VkDescriptorSetLayout>	descriptorSetLayouts;
		VkDescriptorPool					descriptorPool = VK_NULL_HANDLE;
	};

	class DVKShaderModule
	{
	private:
		DVKShaderModule()
		{

		}

	public:
		
		~DVKShaderModule()
		{
			if (handle != VK_NULL_HANDLE) 
			{
				vkDestroyShaderModule(device, handle, VULKAN_CPU_ALLOCATOR);
				handle = VK_NULL_HANDLE;
			}
			
			if (data) 
			{
				delete[] data;
				data = nullptr;
			}
		}

		static DVKShaderModule* Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* filename, VkShaderStageFlagBits stage);
		
	public:

		VkDevice				device;
		VkShaderStageFlagBits	stage;
		VkShaderModule			handle;
		uint8*					data;
		uint32					size;
	};

	class DVKShader
	{
		struct BufferInfo
		{
			uint32				set = 0;
			uint32				binding = 0;
			uint32				bufferSize = 0;
			VkDescriptorType	descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			VkShaderStageFlags	stageFlags = 0;
		};

		struct ImageInfo
		{
			uint32				set = 0;
			uint32				binding = 0;
			VkDescriptorType	descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			VkShaderStageFlags	stageFlags = 0;
		};

	private:
		typedef std::vector<VkPipelineShaderStageCreateInfo>	ShaderStageInfoArray;
		typedef std::vector<VkDescriptorSetLayout>				DescriptorSetLayouts;
		typedef std::vector<DVKDescriptorSetPool*>				DVKDescriptorSetPools;

		DVKShader()
		{

		}

	public:
		~DVKShader()
		{
			if (vertShaderModule) 
			{
				delete vertShaderModule;
				vertShaderModule = nullptr;
			}

			if (fragShaderModule) 
			{
				delete fragShaderModule;
				fragShaderModule = nullptr;
			}

			if (geomShaderModule) 
			{
				delete geomShaderModule;
				geomShaderModule = nullptr;
			}

			if (compShaderModule) 
			{
				delete compShaderModule;
				compShaderModule = nullptr;
			}

			if (tescShaderModule) 
			{
				delete tescShaderModule;
				tescShaderModule = nullptr;
			}

			if (teseShaderModule) 
			{
				delete teseShaderModule;
				teseShaderModule = nullptr;
			}

			for (int32 i = 0; i < descriptorSetLayouts.size(); ++i) {
				vkDestroyDescriptorSetLayout(device, descriptorSetLayouts[i], VULKAN_CPU_ALLOCATOR);
			}
			descriptorSetLayouts.clear();

			if (pipelineLayout != VK_NULL_HANDLE)
			{
				vkDestroyPipelineLayout(device, pipelineLayout, VULKAN_CPU_ALLOCATOR);
				pipelineLayout = VK_NULL_HANDLE;
			}

			for (int32 i = 0; i < descriptorSetPools.size(); ++i) {
				delete descriptorSetPools[i];
			}
			descriptorSetPools.clear();
			
		}

		static DVKShader* Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* comp); 

		static DVKShader* Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* vert, const char* frag, const char* geom = nullptr, const char* comp = nullptr, const char* tesc = nullptr, const char* tese = nullptr);
        
        static DVKShader* Create(std::shared_ptr<VulkanDevice> vulkanDevice, bool dynamicUBO, const char* vert, const char* frag, const char* geom = nullptr, const char* comp = nullptr, const char* tesc = nullptr, const char* tese = nullptr);
        
		DVKDescriptorSet* AllocateDescriptorSet()
		{
			if (setLayoutsInfo.setLayouts.size() == 0) {
				return nullptr;
			}

			DVKDescriptorSet* dvkSet = new DVKDescriptorSet();
			dvkSet->device = device;
			dvkSet->setLayoutsInfo = setLayoutsInfo;
			dvkSet->descriptorSets.resize(setLayoutsInfo.setLayouts.size());

			for (int32 i = descriptorSetPools.size() - 1; i >= 0; --i)
			{
				if (descriptorSetPools[i]->AllocateDescriptorSet(dvkSet->descriptorSets.data())) {
					return dvkSet;
				}
			}

			DVKDescriptorSetPool* setPool = new DVKDescriptorSetPool(device, 64, setLayoutsInfo, descriptorSetLayouts);
			descriptorSetPools.push_back(setPool);
			setPool->AllocateDescriptorSet(dvkSet->descriptorSets.data());

			return dvkSet;
		}

	private:

		void Compile();

		void GenerateLayout();
        
        void GenerateInputInfo();

		void ProcessStorageBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);
        
        void ProcessStorageImages(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);
        
        void ProcessInput(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);
        
        void ProcessTextures(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);

        void ProcessAttachments(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkPipelineStageFlags stageFlags);
        
        void ProcessUniformBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);
        
		void ProcessShaderModule(DVKShaderModule* shaderModule);

	private:
		std::vector<DVKAttribute> m_InputAttributes;

	public:
        
        typedef std::vector<VkVertexInputBindingDescription>    InputBindingsVector;
        typedef std::vector<VkVertexInputAttributeDescription>  InputAttributesVector;

		DVKShaderModule*				vertShaderModule = nullptr;
		DVKShaderModule*				fragShaderModule = nullptr;
		DVKShaderModule*				geomShaderModule = nullptr;
		DVKShaderModule*				compShaderModule = nullptr;
		DVKShaderModule*				tescShaderModule = nullptr;
		DVKShaderModule*				teseShaderModule = nullptr;

		VkDevice						device = VK_NULL_HANDLE;
        bool                            dynamicUBO = false;

		ShaderStageInfoArray			shaderStageCreateInfos;
		DVKDescriptorSetLayoutsInfo		setLayoutsInfo;
		std::vector<VertexAttribute>	perVertexAttributes;
        std::vector<VertexAttribute>    instancesAttributes;
        InputBindingsVector             inputBindings;
        InputAttributesVector           inputAttributes;
        
		DescriptorSetLayouts 			descriptorSetLayouts;
		VkPipelineLayout 				pipelineLayout = VK_NULL_HANDLE;
		DVKDescriptorSetPools			descriptorSetPools;

		std::unordered_map<std::string, BufferInfo>	bufferParams;
		std::unordered_map<std::string, ImageInfo>	imageParams;
	};

}
