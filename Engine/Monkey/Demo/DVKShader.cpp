﻿#include "DVKShader.h"
#include "spirv_cross.hpp"

namespace vk_demo
{
	DVKShaderModule* DVKShaderModule::Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* filename, VkShaderStageFlagBits stage)
	{
		VkDevice device = vulkanDevice->GetInstanceHandle();

		uint8* dataPtr  = nullptr;
        uint32 dataSize = 0;
        if (!FileManager::ReadFile(filename, dataPtr, dataSize))
		{
			MLOGE("Failed load file:%s", filename);
			return nullptr;
		}
        
        VkShaderModuleCreateInfo moduleCreateInfo;
        ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
        moduleCreateInfo.codeSize = dataSize;
        moduleCreateInfo.pCode    = (uint32_t*)dataPtr;
        
        VkShaderModule shaderModule = VK_NULL_HANDLE;
        VERIFYVULKANRESULT(vkCreateShaderModule(device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));

		DVKShaderModule* dvkModule = new DVKShaderModule();
		dvkModule->data   = dataPtr;
		dvkModule->size   = dataSize;
		dvkModule->device = device;
		dvkModule->handle = shaderModule;
		dvkModule->stage  = stage;

		return dvkModule;
	}

	DVKShader* DVKShader::Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* vert, const char* frag, const char* geom, const char* comp, const char* tesc, const char* tese)
	{
		DVKShaderModule* vertModule = vert ? DVKShaderModule::Create(vulkanDevice, vert, VK_SHADER_STAGE_VERTEX_BIT)   : nullptr;
		DVKShaderModule* fragModule = frag ? DVKShaderModule::Create(vulkanDevice, frag, VK_SHADER_STAGE_FRAGMENT_BIT) : nullptr;
		DVKShaderModule* geomModule = geom ? DVKShaderModule::Create(vulkanDevice, geom, VK_SHADER_STAGE_GEOMETRY_BIT) : nullptr;
		DVKShaderModule* compModule = comp ? DVKShaderModule::Create(vulkanDevice, comp, VK_SHADER_STAGE_COMPUTE_BIT) : nullptr;
		DVKShaderModule* tescModule = tesc ? DVKShaderModule::Create(vulkanDevice, tesc, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)    : nullptr;
		DVKShaderModule* teseModule = tese ? DVKShaderModule::Create(vulkanDevice, tese, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) : nullptr;
		
		DVKShader* shader = new DVKShader();

		shader->vertShaderModule = vertModule;
		shader->fragShaderModule = fragModule;
		shader->geomShaderModule = geomModule;
		shader->compShaderModule = compModule;
		shader->tescShaderModule = tescModule;
		shader->teseShaderModule = teseModule;

		shader->device = vulkanDevice->GetInstanceHandle();
		shader->Compile();

		return shader;
	}

	void DVKShader::ProcessShaderModule(DVKShaderModule* shaderModule)
	{
		if (!shaderModule) {
			return;
		}

		// 保存StageInfo
		VkPipelineShaderStageCreateInfo shaderCreateInfo;
		ZeroVulkanStruct(shaderCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		shaderCreateInfo.stage  = shaderModule->stage;
		shaderCreateInfo.module = shaderModule->handle;
		shaderCreateInfo.pName  = "main";
		shaderStageCreateInfos.push_back(shaderCreateInfo);

		// 反编译Shader获取相关信息
		spirv_cross::Compiler compiler((uint32*)shaderModule->data, shaderModule->size / sizeof(uint32));
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		
		// 获取input信息
		for (int32 i = 0; i < resources.subpass_inputs.size(); ++i)
		{
			spirv_cross::Resource& res      = resources.subpass_inputs[i];
			spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
			spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
			const std::string &varName      = compiler.get_name(res.id);

			int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
			int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.binding 			= binding;
			setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			setLayoutBinding.descriptorCount    = 1;
			setLayoutBinding.stageFlags 		= shaderModule->stage;
			setLayoutBinding.pImmutableSamplers = nullptr;

			setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);
		}
		
		// 获取Uniform Buffer信息
		for (int32 i = 0; i < resources.uniform_buffers.size(); ++i)
		{
			spirv_cross::Resource& res      = resources.uniform_buffers[i];
			spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
			spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
			const std::string &varName      = compiler.get_name(res.id);
			const std::string &typeName     = compiler.get_name(res.base_type_id);
			
			int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
			int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

			// [layout (binding = 0) uniform MVPDynamicBlock] 标记为Dynamic的buffer
			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.binding 			= binding;
			setLayoutBinding.descriptorType     = typeName.find("Dynamic") != std::string::npos ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			setLayoutBinding.descriptorCount    = 1;
			setLayoutBinding.stageFlags 		= shaderModule->stage;
			setLayoutBinding.pImmutableSamplers = nullptr;

			setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);
		}
		
		// 获取Texture
		for (int32 i = 0; i < resources.sampled_images.size(); ++i)
		{
			spirv_cross::Resource& res      = resources.sampled_images[i];
			spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
			spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
			const std::string&      varName = compiler.get_name(res.id);

			int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
			int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.binding 			= binding;
			setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			setLayoutBinding.descriptorCount    = 1;
			setLayoutBinding.stageFlags 		= shaderModule->stage;
			setLayoutBinding.pImmutableSamplers = nullptr;

			setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);
		}

		// 获取input信息
		if (shaderModule->stage == VK_SHADER_STAGE_VERTEX_BIT)
		{
			for (int32 i = 0; i < resources.stage_inputs.size(); ++i)
			{
				spirv_cross::Resource& res = resources.stage_inputs[i];
				const std::string &varName = compiler.get_name(res.id);
				VertexAttribute attribute  = StringToVertexAttribute(varName.c_str());

				int32 location = compiler.get_decoration(res.id, spv::DecorationLocation);
				// location必须连续
				DVKAttribute dvkAttribute = {};
				dvkAttribute.location  = location;
				dvkAttribute.attribute = attribute;
				inputAttributes.push_back(dvkAttribute);
			}
		}

	}

	void DVKShader::Compile()
	{
		ProcessShaderModule(vertShaderModule);
		ProcessShaderModule(fragShaderModule);
		ProcessShaderModule(geomShaderModule);
		ProcessShaderModule(compShaderModule);
		ProcessShaderModule(tescShaderModule);
		ProcessShaderModule(teseShaderModule);
		
		// 对inputAttributes进行排序，获取Attributes列表
		std::sort(inputAttributes.begin(), inputAttributes.end(), [](const DVKAttribute& a, const DVKAttribute& b) -> bool {
			return a.location < b.location;
		});

		for (int32 i = 0; i < inputAttributes.size(); ++i) {
			attributes.push_back(inputAttributes[i].attribute);
		}

		// 生成PipelineLayout
		GenerateLayout();
	}

	void DVKShader::GenerateLayout()
	{
		for (int32 i = 0; i < setLayoutsInfo.setLayouts.size(); ++i)
		{
			VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
			DVKDescriptorSetLayoutInfo& setLayoutInfo = setLayoutsInfo.setLayouts[i];
			
			VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
			ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
			descSetLayoutInfo.bindingCount = setLayoutInfo.bindings.size();
			descSetLayoutInfo.pBindings    = setLayoutInfo.bindings.data();
			VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &descriptorSetLayout));

			descriptorSetLayouts.push_back(descriptorSetLayout);
		}

		VkPipelineLayoutCreateInfo pipeLayoutInfo;
		ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipeLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
		pipeLayoutInfo.pSetLayouts    = descriptorSetLayouts.data();
		VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &pipelineLayout));
	}
	
};