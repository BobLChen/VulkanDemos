#include "DVKShader.h"
#include "DVKVertexBuffer.h"
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
    
    DVKShader* DVKShader::Create(std::shared_ptr<VulkanDevice> vulkanDevice, bool dynamicUBO, const char* vert, const char* frag, const char* geom, const char* comp, const char* tesc, const char* tese)
    {
        DVKShaderModule* vertModule = vert ? DVKShaderModule::Create(vulkanDevice, vert, VK_SHADER_STAGE_VERTEX_BIT)   : nullptr;
        DVKShaderModule* fragModule = frag ? DVKShaderModule::Create(vulkanDevice, frag, VK_SHADER_STAGE_FRAGMENT_BIT) : nullptr;
        DVKShaderModule* geomModule = geom ? DVKShaderModule::Create(vulkanDevice, geom, VK_SHADER_STAGE_GEOMETRY_BIT) : nullptr;
        DVKShaderModule* compModule = comp ? DVKShaderModule::Create(vulkanDevice, comp, VK_SHADER_STAGE_COMPUTE_BIT) : nullptr;
        DVKShaderModule* tescModule = tesc ? DVKShaderModule::Create(vulkanDevice, tesc, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)    : nullptr;
        DVKShaderModule* teseModule = tese ? DVKShaderModule::Create(vulkanDevice, tese, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) : nullptr;
        
        DVKShader* shader = new DVKShader();
        shader->device     = vulkanDevice->GetInstanceHandle();
        shader->dynamicUBO = dynamicUBO;
        
        shader->vertShaderModule = vertModule;
        shader->fragShaderModule = fragModule;
        shader->geomShaderModule = geomModule;
        shader->compShaderModule = compModule;
        shader->tescShaderModule = tescModule;
        shader->teseShaderModule = teseModule;
        
        shader->Compile();
        
        return shader;
    }

	DVKShader* DVKShader::Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* comp)
	{
		return Create(vulkanDevice, true, nullptr, nullptr, nullptr, comp, nullptr, nullptr);
	}

	DVKShader* DVKShader::Create(std::shared_ptr<VulkanDevice> vulkanDevice, const char* vert, const char* frag, const char* geom, const char* comp, const char* tesc, const char* tese)
	{
        return Create(vulkanDevice, false, vert, frag, geom, comp, tesc, tese);
	}

    void DVKShader::ProcessAttachments(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkPipelineStageFlags stageFlags)
    {
        // 获取attachment信息
        for (int32 i = 0; i < resources.subpass_inputs.size(); ++i)
        {
            spirv_cross::Resource& res      = resources.subpass_inputs[i];
            spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
            spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
            const std::string &varName      = compiler.get_name(res.id);
            
            int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            
            VkDescriptorSetLayoutBinding setLayoutBinding = {};
            setLayoutBinding.binding             = binding;
            setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
            setLayoutBinding.descriptorCount    = 1;
            setLayoutBinding.stageFlags         = stageFlags;
            setLayoutBinding.pImmutableSamplers = nullptr;
            
            setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);
            
			auto it = imageParams.find(varName);
			if (it == imageParams.end())
			{
				ImageInfo imageInfo = {};
				imageInfo.set            = set;
				imageInfo.binding        = binding;
				imageInfo.stageFlags     = stageFlags;
				imageInfo.descriptorType = setLayoutBinding.descriptorType;
				imageParams.insert(std::make_pair(varName, imageInfo));
			}
			else
			{
				it->second.stageFlags |= stageFlags;
			}

        }
    }
    
    void DVKShader::ProcessUniformBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
    {
        // 获取Uniform Buffer信息
        for (int32 i = 0; i < resources.uniform_buffers.size(); ++i)
        {
            spirv_cross::Resource& res      = resources.uniform_buffers[i];
            spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
            spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
            const std::string &varName      = compiler.get_name(res.id);
            const std::string &typeName     = compiler.get_name(res.base_type_id);
            uint32 uniformBufferStructSize  = compiler.get_declared_struct_size(type);
            
            int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            
            // [layout (binding = 0) uniform MVPDynamicBlock] 标记为Dynamic的buffer
            VkDescriptorSetLayoutBinding setLayoutBinding = {};
            setLayoutBinding.binding             = binding;
            setLayoutBinding.descriptorType     = (typeName.find("Dynamic") != std::string::npos || dynamicUBO) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            setLayoutBinding.descriptorCount    = 1;
            setLayoutBinding.stageFlags         = stageFlags;
            setLayoutBinding.pImmutableSamplers = nullptr;
            
            setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);
            
            // 保存UBO变量信息
            auto it = bufferParams.find(varName);
            if (it == bufferParams.end())
            {
                BufferInfo bufferInfo = {};
				bufferInfo.set            = set;
				bufferInfo.binding        = binding;
				bufferInfo.bufferSize     = uniformBufferStructSize;
				bufferInfo.stageFlags     = stageFlags;
				bufferInfo.descriptorType = setLayoutBinding.descriptorType;
				bufferParams.insert(std::make_pair(varName, bufferInfo));
            }
            else
            {
                it->second.stageFlags |= setLayoutBinding.stageFlags;
            }
        }
    }
    
    void DVKShader::ProcessTextures(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
    {
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
            setLayoutBinding.binding             = binding;
            setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            setLayoutBinding.descriptorCount    = 1;
            setLayoutBinding.stageFlags         = stageFlags;
            setLayoutBinding.pImmutableSamplers = nullptr;
            
            setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);
            
			auto it = imageParams.find(varName);
			if (it == imageParams.end())
			{
				ImageInfo imageInfo = {};
				imageInfo.set            = set;
				imageInfo.binding        = binding;
				imageInfo.stageFlags     = stageFlags;
				imageInfo.descriptorType = setLayoutBinding.descriptorType;
				imageParams.insert(std::make_pair(varName, imageInfo));
			}
			else
			{
				it->second.stageFlags |= stageFlags;
			}
        }
    }
    
    void DVKShader::ProcessInput(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
    {
        if (stageFlags != VK_SHADER_STAGE_VERTEX_BIT) {
            return;
        }

        // 获取input信息
        for (int32 i = 0; i < resources.stage_inputs.size(); ++i)
        {
            spirv_cross::Resource& res = resources.stage_inputs[i];
            spirv_cross::SPIRType type = compiler.get_type(res.type_id);
            const std::string &varName = compiler.get_name(res.id);
            int32 inputAttributeSize   = type.vecsize;
            
            VertexAttribute attribute  = StringToVertexAttribute(varName.c_str());
            if (attribute == VertexAttribute::VA_None)
            {
                if (inputAttributeSize == 1) {
                    attribute = VertexAttribute::VA_InstanceFloat1;
                }
                else if (inputAttributeSize == 2) {
                    attribute = VertexAttribute::VA_InstanceFloat2;
                }
                else if (inputAttributeSize == 3) {
                    attribute = VertexAttribute::VA_InstanceFloat3;
                }
                else if (inputAttributeSize == 4) {
                    attribute = VertexAttribute::VA_InstanceFloat4;
                }
                MLOG("Not found attribute : %s, treat as instance attribute : %d.", varName.c_str(), int32(attribute));
            }
            
			// location必须连续
            int32 location = compiler.get_decoration(res.id, spv::DecorationLocation);
            DVKAttribute dvkAttribute = {};
            dvkAttribute.location  = location;
            dvkAttribute.attribute = attribute;
            m_InputAttributes.push_back(dvkAttribute);
        }
    }
    
	void DVKShader::ProcessStorageBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
	{
		for (int32 i = 0; i < resources.storage_buffers.size(); ++i)
		{
			spirv_cross::Resource& res      = resources.storage_buffers[i];
			spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
			spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
			const std::string &varName      = compiler.get_name(res.id);

			int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
			int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

			VkDescriptorSetLayoutBinding setLayoutBinding = {};
			setLayoutBinding.binding            = binding;
			setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			setLayoutBinding.descriptorCount    = 1;
			setLayoutBinding.stageFlags         = stageFlags;
			setLayoutBinding.pImmutableSamplers = nullptr;

			setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

			// 保存UBO变量信息
			auto it = bufferParams.find(varName);
			if (it == bufferParams.end())
			{
				BufferInfo bufferInfo = {};
				bufferInfo.set            = set;
				bufferInfo.binding        = binding;
				bufferInfo.bufferSize     = 0;
				bufferInfo.stageFlags     = stageFlags;
				bufferInfo.descriptorType = setLayoutBinding.descriptorType;
				bufferParams.insert(std::make_pair(varName, bufferInfo));
			}
			else
			{
				it->second.stageFlags = it->second.stageFlags | setLayoutBinding.stageFlags;
			}
		}
	}

    void DVKShader::ProcessStorageImages(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags)
    {
        for (int32 i = 0; i < resources.storage_images.size(); ++i)
        {
            spirv_cross::Resource& res      = resources.storage_images[i];
            spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
            spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
            const std::string&      varName = compiler.get_name(res.id);
            
            int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
            int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);
            
            VkDescriptorSetLayoutBinding setLayoutBinding = {};
            setLayoutBinding.binding             = binding;
            setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            setLayoutBinding.descriptorCount    = 1;
            setLayoutBinding.stageFlags         = stageFlags;
            setLayoutBinding.pImmutableSamplers = nullptr;
            
            setLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);
            
			auto it = imageParams.find(varName);
			if (it == imageParams.end())
			{
				ImageInfo imageInfo = {};
				imageInfo.set            = set;
				imageInfo.binding        = binding;
				imageInfo.stageFlags     = stageFlags;
				imageInfo.descriptorType = setLayoutBinding.descriptorType;
				imageParams.insert(std::make_pair(varName, imageInfo));
			}
			else
			{
				it->second.stageFlags |= stageFlags;
			}
            
        }
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
		
        ProcessAttachments(compiler, resources, shaderModule->stage);
        ProcessUniformBuffers(compiler, resources, shaderModule->stage);
        ProcessTextures(compiler, resources, shaderModule->stage);
        ProcessStorageImages(compiler, resources, shaderModule->stage);
        ProcessInput(compiler, resources, shaderModule->stage);
		ProcessStorageBuffers(compiler, resources, shaderModule->stage);

	}

	void DVKShader::Compile()
	{
		ProcessShaderModule(vertShaderModule);
		ProcessShaderModule(fragShaderModule);
		ProcessShaderModule(geomShaderModule);
		ProcessShaderModule(compShaderModule);
		ProcessShaderModule(tescShaderModule);
		ProcessShaderModule(teseShaderModule);
        GenerateInputInfo();
		GenerateLayout();
	}
    
    void DVKShader::GenerateInputInfo()
    {
        // 对inputAttributes进行排序，获取Attributes列表
        std::sort(m_InputAttributes.begin(), m_InputAttributes.end(), [](const DVKAttribute& a, const DVKAttribute& b) -> bool {
            return a.location < b.location;
        });
        
        // 对inputAttributes进行归类整理
        for (int32 i = 0; i < m_InputAttributes.size(); ++i)
        {
            VertexAttribute attribute = m_InputAttributes[i].attribute;
            if (attribute == VA_InstanceFloat1 || attribute == VA_InstanceFloat2 || attribute == VA_InstanceFloat3 || attribute == VA_InstanceFloat4) {
                instancesAttributes.push_back(attribute);
            }
            else {
                perVertexAttributes.push_back(attribute);
            }
        }
        
        // 生成Bindinfo
        inputBindings.resize(0);
        if (perVertexAttributes.size() > 0)
        {
            int32 stride = 0;
            for (int32 i = 0; i < perVertexAttributes.size(); ++i) {
                stride += VertexAttributeToSize(perVertexAttributes[i]);
            }
            VkVertexInputBindingDescription perVertexInputBinding = {};
            perVertexInputBinding.binding   = 0;
            perVertexInputBinding.stride    = stride;
            perVertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            inputBindings.push_back(perVertexInputBinding);
        }
        
        if (instancesAttributes.size() > 0)
        {
            int32 stride = 0;
            for (int32 i = 0; i < instancesAttributes.size(); ++i) {
                stride += VertexAttributeToSize(instancesAttributes[i]);
            }
            VkVertexInputBindingDescription instanceInputBinding = {};
            instanceInputBinding.binding   = 1;
            instanceInputBinding.stride    = stride;
            instanceInputBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
            inputBindings.push_back(instanceInputBinding);
        }
        
        // 生成attributes info
        int location = 0;
        if (perVertexAttributes.size() > 0)
        {
            int32 offset = 0;
            for (int32 i = 0; i < perVertexAttributes.size(); ++i)
            {
                VkVertexInputAttributeDescription inputAttribute = {};
                inputAttribute.binding  = 0;
                inputAttribute.location = location;
                inputAttribute.format   = VertexAttributeToVkFormat(perVertexAttributes[i]);
                inputAttribute.offset   = offset;
                offset += VertexAttributeToSize(perVertexAttributes[i]);
                inputAttributes.push_back(inputAttribute);
                
                location += 1;
            }
        }
        
        if (instancesAttributes.size() > 0)
        {
            int32 offset = 0;
            for (int32 i = 0; i < instancesAttributes.size(); ++i)
            {
                VkVertexInputAttributeDescription inputAttribute = {};
                inputAttribute.binding  = 1;
                inputAttribute.location = location;
                inputAttribute.format   = VertexAttributeToVkFormat(instancesAttributes[i]);
                inputAttribute.offset   = offset;
                offset += VertexAttributeToSize(instancesAttributes[i]);
                inputAttributes.push_back(inputAttribute);
                
                location += 1;
            }
        }
        
    }
    
	void DVKShader::GenerateLayout()
	{
        std::vector<DVKDescriptorSetLayoutInfo>& setLayouts = setLayoutsInfo.setLayouts;
        
        // 先按照set进行排序
        std::sort(setLayouts.begin(), setLayouts.end(), [](const DVKDescriptorSetLayoutInfo& a, const DVKDescriptorSetLayoutInfo& b) -> bool {
            return a.set < b.set;
        });
        
        // 再按照binding进行排序
        for (int32 i = 0; i < setLayouts.size(); ++i)
        {
            std::vector<VkDescriptorSetLayoutBinding>& bindings = setLayouts[i].bindings;
            std::sort(bindings.begin(), bindings.end(), [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) -> bool {
                return a.binding < b.binding;
            });
        }
        
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
