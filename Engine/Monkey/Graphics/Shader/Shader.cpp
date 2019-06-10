#include "Shader.h"
#include "Engine.h"
#include "spirv_cross.hpp"

#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanMemory.h"
#include "Vulkan/VulkanDevice.h"

#include "File/FileManager.h"

std::unordered_map<std::string, std::shared_ptr<ShaderModule>> Shader::g_ShaderModules;

ShaderModule::~ShaderModule()
{
	if (m_ShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle(), m_ShaderModule, VULKAN_CPU_ALLOCATOR);
		m_ShaderModule = VK_NULL_HANDLE;
	}
    
    if (m_Data)
    {
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
	if (it != g_ShaderModules.end())
	{
		return it->second;
	}

	uint32 dataSize = 0;
	uint8* dataPtr  = nullptr;
	if (!FileManager::ReadFile(filename, dataPtr, dataSize))
	{
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
	uint32 temp = 0;
	if (vert) {
		temp = vert->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (frag) {
		temp = frag->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (geom) {
		temp = geom->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (comp) {
		temp = comp->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (tesc) {
		temp = tesc->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	if (tese) {
		temp = tese->GetHash();
		m_Hash = Crc::MemCrc32(&temp, sizeof(uint32));
	}

	ProcessBindingsForStage(m_VertShaderModule);
	ProcessBindingsForStage(m_FragShaderModule);
	ProcessBindingsForStage(m_GeomShaderModule);
	ProcessBindingsForStage(m_CompShaderModule);
	ProcessBindingsForStage(m_TescShaderModule);
	ProcessBindingsForStage(m_TeseShaderModule);
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

void Shader::ProcessBindingsForStage(std::shared_ptr<ShaderModule> shaderModule)
{
	// 反编译Shader获取相关信息
	spirv_cross::Compiler compiler(shaderModule->GetData(), shaderModule->GetDataSize() / sizeof(uint32));
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();
	
	// push constant不支持

    // 获取Uniform Buffer信息
    for (int32 i = 0; i < resources.uniform_buffers.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.uniform_buffers[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string &varName      = compiler.get_name(res.id);

        int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
		int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

		// 直接使用Dynamic的UniformBuffer
		m_SetsLayoutInfo.AddDescriptor(set, binding, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, shaderModule->GetStageFlags());
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

		m_SetsLayoutInfo.AddDescriptor(set, binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, shaderModule->GetStageFlags());
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
	return shader;
}

void Shader::UpdateVertPipelineLayout()
{
    if (m_VertShaderModule == nullptr)
    {
        return;
    }

	// 保存StageInfo
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
	stageInfo.module = m_VertShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
    
	// 反编译Shader获取相关信息
    spirv_cross::Compiler compiler(m_VertShaderModule->GetData(), m_VertShaderModule->GetDataSize() / sizeof(uint32));
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    CollectResources(compiler, resources, VK_SHADER_STAGE_VERTEX_BIT);
    
	// 获取Input Location信息
    for (int32 i = 0; i < resources.stage_inputs.size(); ++i)
    {
        spirv_cross::Resource& res = resources.stage_inputs[i];
        const std::string &varName = compiler.get_name(res.id);
        VertexAttribute attribute  = StringToVertexAttribute(varName.c_str());
		m_VertexInputBindingInfo.AddBinding(attribute, compiler.get_decoration(res.id, spv::DecorationLocation));
    }

	m_VertexInputBindingInfo.GenerateHash();
}

void Shader::UpdatePipelineLayout()
{
	if (!m_InvalidLayout) 
	{
		return;
	}

    DestroyPipelineLayout();

    UpdateVertPipelineLayout();
	UpdateGeomPipelineLayout();
	UpdateTescPipelineLayout();
	UpdateTesePipelineLayout();
	UpdateCompPipelineLayout();
    UpdateFragPipelineLayout();
    
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    // 创建SetLayout
    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
    ZeroVulkanStruct(setLayoutCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
    setLayoutCreateInfo.bindingCount = uint32_t(m_SetLayoutBindings.size());
    setLayoutCreateInfo.pBindings    = m_SetLayoutBindings.data();
    VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));
    
	// 创建PipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    ZeroVulkanStruct(pipelineLayoutCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts    = &m_DescriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)m_PushConstantRanges.size();
    pipelineLayoutCreateInfo.pPushConstantRanges    = m_PushConstantRanges.data();
    VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
    
	// PoolSize
	for (int32 i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i)
	{
		if (m_DescriptorTypes[i] > 0)
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.descriptorCount = m_DescriptorTypes[i];
			poolSize.type = (VkDescriptorType)i;
			m_PoolSizes.push_back(poolSize);
		}
	}

	m_InvalidLayout  = false;
}
