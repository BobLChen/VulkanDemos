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