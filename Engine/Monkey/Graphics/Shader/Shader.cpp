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

std::shared_ptr<ShaderModule> Shader::LoadSPIPVShader(const std::string& filename)
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
    
	return std::make_shared<ShaderModule>(shaderModule, (uint32_t*)dataPtr, dataSize);
}

Shader::Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom, std::shared_ptr<ShaderModule> comp, std::shared_ptr<ShaderModule> tesc, std::shared_ptr<ShaderModule> tese)
	: m_Hash(0)
    , m_InvalidLayout(true)
	, m_PipelineLayout(VK_NULL_HANDLE)
	, m_VertShaderModule(vert)
	, m_FragShaderModule(frag)
	, m_GeomShaderModule(geom)
	, m_CompShaderModule(comp)
	, m_TescShaderModule(tesc)
	, m_TeseShaderModule(tese)
{
    uint32 hash0 = Crc::MakeHashCode(
        vert != nullptr ? vert->GetHash() : 0,
        frag != nullptr ? frag->GetHash() : 0,
        geom != nullptr ? geom->GetHash() : 0
    );
    uint32 hash1 = Crc::MakeHashCode(
        comp != nullptr ? comp->GetHash() : 0,
        tesc != nullptr ? tesc->GetHash() : 0,
        tese != nullptr ? tese->GetHash() : 0
    );
    m_Hash = Crc::MakeHashCode(hash0, hash1);

	std::memset(m_DescriptorTypes, 0, sizeof(m_DescriptorTypes));
}

Shader::~Shader()
{
	m_VertShaderModule = nullptr;
	m_FragShaderModule = nullptr;
	m_GeomShaderModule = nullptr;
	m_CompShaderModule = nullptr;
	m_TescShaderModule = nullptr;
	m_TeseShaderModule = nullptr;

    DestroyPipelineLayout();
}

std::shared_ptr<Shader> Shader::Create(const char* vert, const char* frag, const char* geom, const char* compute, const char* tesc, const char* tese)
{
	std::shared_ptr<ShaderModule> vertModule = vert ? LoadSPIPVShader(vert) : nullptr;
	std::shared_ptr<ShaderModule> fragModule = frag ? LoadSPIPVShader(frag) : nullptr;
	std::shared_ptr<ShaderModule> geomModule = geom ? LoadSPIPVShader(geom) : nullptr;
	std::shared_ptr<ShaderModule> tescModule = tesc ? LoadSPIPVShader(tesc) : nullptr;
	std::shared_ptr<ShaderModule> teseModule = tese ? LoadSPIPVShader(tese) : nullptr;
	return std::make_shared<Shader>(vertModule, fragModule, geomModule, tescModule, teseModule);
}

void Shader::DestroyPipelineLayout()
{
	if (m_InvalidLayout)
	{
		return;
	}
	m_InvalidLayout  = true;
    
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
    vkDestroyPipelineLayout(device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
	
	m_PoolSizes.clear();
    m_ShaderStages.clear();
    m_SetLayoutBindings.clear();
	m_VertexInputBindingInfo.Clear();
}

void Shader::UpdateFragPipelineLayout()
{
	if (m_FragShaderModule == nullptr)
	{
		return;
	}

	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	stageInfo.module = m_FragShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);

	// 反编译Shader获取相关信息
	spirv_cross::Compiler compiler(m_FragShaderModule->GetData(), m_FragShaderModule->GetDataSize() / sizeof(uint32));
	spirv_cross::ShaderResources resources = compiler.get_shader_resources();

	// 获取Texture
	for (int32 i = 0; i < resources.sampled_images.size(); ++i)
	{
		spirv_cross::Resource& res = resources.sampled_images[i];
		spirv_cross::SPIRType type = compiler.get_type(res.type_id);
		spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
		const std::string &varName = compiler.get_name(res.id);
        int32 bindingSet = compiler.get_decoration(res.id, spv::DecorationBinding);
        int32 bindingIdx = -1;
        
        for (int32 j = 0; j < m_SetLayoutBindings.size(); ++j)
        {
            if (m_SetLayoutBindings[j].binding == bindingSet &&
                m_SetLayoutBindings[j].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER &&
                m_SetLayoutBindings[j].stageFlags == VK_SHADER_STAGE_FRAGMENT_BIT)
            {
                bindingIdx = j;
                break;
            }
        }
        
        if (bindingIdx != -1)
        {
            m_SetLayoutBindings[bindingIdx].descriptorCount += 1;
        }
        else
        {
            VkDescriptorSetLayoutBinding bindingInfo = {};
            bindingInfo.binding            = bindingSet;
            bindingInfo.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            bindingInfo.descriptorCount    = 1;
            bindingInfo.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
            bindingInfo.pImmutableSamplers = nullptr;
            m_SetLayoutBindings.push_back(bindingInfo);
        }
		
		m_DescriptorTypes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] += 1;
	}
    
}

void Shader::UpdateCompPipelineLayout()
{
	if (m_CompShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo.module = m_CompShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateGeomPipelineLayout()
{
	if (m_GeomShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
	stageInfo.module = m_GeomShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateTescPipelineLayout()
{
	if (m_TescShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	stageInfo.module = m_TescShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateTesePipelineLayout()
{
	if (m_TeseShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	stageInfo.module = m_TeseShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
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
    
	// 获取Uniform Buffer信息
    for (int32 i = 0; i < resources.uniform_buffers.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.uniform_buffers[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string &varName      = compiler.get_name(res.id);
        int32 bindingSet = compiler.get_decoration(res.id, spv::DecorationBinding);
        int32 bindingIdx = -1;
        
        for (int32 j = 0; j < m_SetLayoutBindings.size(); ++j)
        {
            if (m_SetLayoutBindings[j].binding == bindingSet &&
                m_SetLayoutBindings[j].descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER &&
                m_SetLayoutBindings[j].stageFlags == VK_SHADER_STAGE_VERTEX_BIT)
            {
                bindingIdx = j;
                break;
            }
        }
        
        if (bindingIdx != -1)
        {
            m_SetLayoutBindings[bindingIdx].descriptorCount += 1;
        }
        else
        {
            VkDescriptorSetLayoutBinding bindingIndo = {};
            bindingIndo.binding = bindingSet;
            bindingIndo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            bindingIndo.descriptorCount = 1;
            bindingIndo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
            bindingIndo.pImmutableSamplers = nullptr;
            m_SetLayoutBindings.push_back(bindingIndo);
        }
        
		m_DescriptorTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] += 1;
    }
    
	// 获取Input Location信息
    for (int32 i = 0; i < resources.stage_inputs.size(); ++i)
    {
        spirv_cross::Resource& res = resources.stage_inputs[i];
        const std::string &varName = compiler.get_name(res.id);
        VertexAttribute attribute  = StringToVertexAttribute(varName.c_str());
		m_VertexInputBindingInfo.AddBinding(attribute, compiler.get_decoration(res.id, spv::DecorationLocation));
    }

	m_VertexInputBindingInfo.Update();
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
