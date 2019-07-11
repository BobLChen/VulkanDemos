#include "Shader.h"
#include "Engine.h"
#include "spirv_cross.hpp"

#include "File/FileManager.h"

#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanMemory.h"
#include "Vulkan/VulkanDevice.h"

std::unordered_map<std::string, std::shared_ptr<ShaderModule>> Shader::g_ShaderModules;

ShaderModule::~ShaderModule()
{
	if (m_ShaderModule != VK_NULL_HANDLE) {
		vkDestroyShaderModule(Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle(), m_ShaderModule, VULKAN_CPU_ALLOCATOR);
		m_ShaderModule = VK_NULL_HANDLE;
	}
    
    if (m_Data) {
        delete[] m_Data;
        m_Data = nullptr;
    }
}

std::shared_ptr<ShaderModule> Shader::LoadSPIPVShader(const std::string& filename, VkShaderStageFlagBits stageFlags)
{
	return Shader::LoadSPIPVShader(filename.c_str(), stageFlags);
}

std::shared_ptr<ShaderModule> Shader::LoadSPIPVShader(const char* filename, VkShaderStageFlagBits stageFlags)
{
	auto it = g_ShaderModules.find(filename);
	if (it != g_ShaderModules.end()) {
		return it->second;
	}

	uint32 dataSize = 0;
	uint8* dataPtr  = nullptr;
	if (!FileManager::ReadFile(filename, dataPtr, dataSize)) {
		return nullptr;
	}

	VkShaderModuleCreateInfo moduleCreateInfo;
	ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
	moduleCreateInfo.codeSize = dataSize;
	moduleCreateInfo.pCode    = (uint32_t*)dataPtr;

	VkShaderModule shaderModule;
	VERIFYVULKANRESULT(vkCreateShaderModule(Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle(), &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
    
	return std::make_shared<ShaderModule>(stageFlags, shaderModule, (uint32_t*)dataPtr, dataSize);
}

Shader::Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom, std::shared_ptr<ShaderModule> comp, std::shared_ptr<ShaderModule> tesc, std::shared_ptr<ShaderModule> tese)
	: m_Hash(0)
	, m_VertShaderModule(vert)
	, m_FragShaderModule(frag)
	, m_GeomShaderModule(geom)
	, m_CompShaderModule(comp)
	, m_TescShaderModule(tesc)
	, m_TeseShaderModule(tese)
{
	
}

Shader::~Shader()
{
	m_VertShaderModule = nullptr;
	m_FragShaderModule = nullptr;
	m_GeomShaderModule = nullptr;
	m_CompShaderModule = nullptr;
	m_TescShaderModule = nullptr;
	m_TeseShaderModule = nullptr;
}

void Shader::ProcessShaderModule(std::shared_ptr<ShaderModule> shaderModule)
{
	if (shaderModule == nullptr) {
		return;
	}

	VkPipelineShaderStageCreateInfo shaderCreateInfo;
    ZeroVulkanStruct(shaderCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    shaderCreateInfo.stage  = shaderModule->GetStageFlags();
    shaderCreateInfo.module = shaderModule->GetHandle();
    shaderCreateInfo.pName  = "main";
    m_ShaderCreateInfos.push_back(shaderCreateInfo);

	// 反编译Shader获取相关信息
	spirv_cross::Compiler compiler(shaderModule->GetData(), shaderModule->GetDataSize() / sizeof(uint32));
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();
	
    // 获取Uniform Buffer信息
    for (int32 i = 0; i < resources.uniform_buffers.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.uniform_buffers[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string &varName      = compiler.get_name(res.id);
		uint32 uniformBufferStructSize  = compiler.get_declared_struct_size(type);
		
        int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

		int32 descIdx = m_SetsLayoutInfo.AddDescriptor(set, binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, shaderModule->GetStageFlags());
		
		ShaderParamInfo paramInfo;
		paramInfo.name            = varName;
		paramInfo.set		      = set;
		paramInfo.binding	      = binding;
		paramInfo.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		paramInfo.stageFlags	  = shaderModule->GetStageFlags();
		paramInfo.bufferSize      = uniformBufferStructSize;
		paramInfo.descriptorIndex = descIdx;
		m_Params.push_back(paramInfo);
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

		int32 descIdx = m_SetsLayoutInfo.AddDescriptor(set, binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, shaderModule->GetStageFlags());
		
		ShaderParamInfo paramInfo;
		paramInfo.name            = varName;
		paramInfo.set		      = set;
		paramInfo.binding	      = binding;
		paramInfo.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		paramInfo.stageFlags	  = shaderModule->GetStageFlags();
		paramInfo.bufferSize      = 0;
		paramInfo.descriptorIndex = descIdx;

		m_Params.push_back(paramInfo);
    }

	// 获取input信息
	if (shaderModule->GetStageFlags() == VK_SHADER_STAGE_VERTEX_BIT)
	{
		for (int32 i = 0; i < resources.stage_inputs.size(); ++i)
		{
			spirv_cross::Resource& res = resources.stage_inputs[i];
			const std::string &varName = compiler.get_name(res.id);
			VertexAttribute attribute  = StringToVertexAttribute(varName.c_str());
			m_VertexInputBindingInfo.AddBinding(attribute, compiler.get_decoration(res.id, spv::DecorationLocation));
		}
		m_VertexInputBindingInfo.GenerateHash();
	}
}

void Shader::Compile()
{
	GenerateHash();

	std::vector<std::shared_ptr<ShaderModule>> shaderModules;
    shaderModules.push_back(m_VertShaderModule);
    shaderModules.push_back(m_FragShaderModule);
    shaderModules.push_back(m_GeomShaderModule);
    shaderModules.push_back(m_TescShaderModule);
    shaderModules.push_back(m_TeseShaderModule);
    shaderModules.push_back(m_CompShaderModule);

    for (int32 i = 0; i < shaderModules.size(); ++i) {
		ProcessShaderModule(shaderModules[i]);
    }

	m_SetsLayoutInfo.Compile();
}

void Shader::GenerateHash()
{
	uint32 temp = 0;
	if (m_VertShaderModule) {
		temp = m_VertShaderModule->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (m_FragShaderModule) {
		temp = m_FragShaderModule->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (m_GeomShaderModule) {
		temp = m_GeomShaderModule->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (m_CompShaderModule) {
		temp = m_CompShaderModule->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (m_TescShaderModule) {
		temp = m_TescShaderModule->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (m_TeseShaderModule) {
		temp = m_TeseShaderModule->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}
}

std::shared_ptr<Shader> Shader::Create(const char* vert, const char* frag, const char* geom, const char* compute, const char* tesc, const char* tese)
{
	std::shared_ptr<ShaderModule> vertModule = vert ? LoadSPIPVShader(vert, VK_SHADER_STAGE_VERTEX_BIT)   : nullptr;
	std::shared_ptr<ShaderModule> fragModule = frag ? LoadSPIPVShader(frag, VK_SHADER_STAGE_FRAGMENT_BIT) : nullptr;
	std::shared_ptr<ShaderModule> geomModule = geom ? LoadSPIPVShader(geom, VK_SHADER_STAGE_GEOMETRY_BIT) : nullptr;
	std::shared_ptr<ShaderModule> tescModule = tesc ? LoadSPIPVShader(tesc, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)    : nullptr;
	std::shared_ptr<ShaderModule> teseModule = tese ? LoadSPIPVShader(tese, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) : nullptr;
	std::shared_ptr<Shader> shader = std::make_shared<Shader>(vertModule, fragModule, geomModule, tescModule, teseModule);
	shader->Compile();

	return shader;
}
