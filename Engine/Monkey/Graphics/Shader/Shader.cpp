#include "Shader.h"
#include "Engine.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanMemory.h"
#include "Vulkan/VulkanDevice.h"
#include "File/FileManager.h"

#include "spirv_cross.hpp"

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
	uint8* dataPtr = nullptr;
	if (!FileManager::ReadFile(filename, dataPtr, dataSize))
	{
		return nullptr;
	}

	VkShaderModuleCreateInfo moduleCreateInfo;
	ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
	moduleCreateInfo.codeSize = dataSize;
	moduleCreateInfo.pCode = (uint32_t*)dataPtr;

	VkShaderModule shaderModule;
	VERIFYVULKANRESULT(vkCreateShaderModule(Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle(), &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
    
	return std::make_shared<ShaderModule>(shaderModule, (uint32_t*)dataPtr, dataSize);
}

Shader::Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom, std::shared_ptr<ShaderModule> comp, std::shared_ptr<ShaderModule> tesc, std::shared_ptr<ShaderModule> tese)
	: m_PipelineLayout(VK_NULL_HANDLE)
	, m_VertShaderModule(vert)
	, m_FragShaderModule(frag)
	, m_GeomShaderModule(geom)
	, m_CompShaderModule(comp)
	, m_TescShaderModule(tesc)
	, m_TeseShaderModule(tese)
	, m_Invalid(true)
    , m_DescriptorPool(VK_NULL_HANDLE)
    , m_DescriptorSetLayout(VK_NULL_HANDLE)
    , m_DescriptorSet(VK_NULL_HANDLE)
    , m_Uploaded(false)
{
    
}

Shader::~Shader()
{
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
    if (!m_Uploaded)
    {
        return;
    }
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
    vkDestroyDescriptorPool(device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
    vkDestroyPipelineLayout(device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
    
    for (int32 i = 0; i < m_UniformBuffers.size(); ++i)
    {
        vkFreeMemory(device, m_UniformBuffers[i].memory, VULKAN_CPU_ALLOCATOR);
        vkDestroyBuffer(device, m_UniformBuffers[i].buffer, VULKAN_CPU_ALLOCATOR);
    }
    m_UniformBuffers.clear();
    
    m_Uploaded = false;
}

void Shader::CreateUniformBuffer(UniformBuffer& uniformBuffer, uint32 dataSize, VkBufferUsageFlags usage)
{
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    VkBufferCreateInfo bufferCreateInfo;
    ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    bufferCreateInfo.size  = dataSize;
    bufferCreateInfo.usage = usage;
    VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &uniformBuffer.buffer));
    
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, uniformBuffer.buffer, &memReqs);
    uint32 memoryTypeIndex = 0;
    Engine::Get()->GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
    
    VkMemoryAllocateInfo allocInfo = {};
    ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    allocInfo.allocationSize  = memReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VERIFYVULKANRESULT(vkAllocateMemory(device, &allocInfo, VULKAN_CPU_ALLOCATOR, &uniformBuffer.memory));
    VERIFYVULKANRESULT(vkBindBufferMemory(device, uniformBuffer.buffer, uniformBuffer.memory, 0));
    
    uniformBuffer.size = dataSize;
    uniformBuffer.allocationSize = memReqs.size;
    uniformBuffer.offset = 0;
}

void Shader::UpdateFragPipelineLayout()
{
    
}

void Shader::UpdateVertPipelineLayout()
{
    if (m_VertShaderModule == nullptr)
    {
        return;
    }
    
    std::shared_ptr<ShaderModule> module = m_VertShaderModule;
    
    spirv_cross::Compiler compiler(module->GetData(), module->GetDataSize() / sizeof(uint32));
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    
    for (int32 i = 0; i < resources.uniform_buffers.size(); ++i)
    {
        spirv_cross::Resource& res = resources.uniform_buffers[i];
        spirv_cross::SPIRType type = compiler.get_type(res.type_id);
        spirv_cross::Bitset mask = compiler.get_decoration_bitset(res.id);
        // uint32 set = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        uint32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        uint32 blockSize = compiler.get_declared_struct_size(base_type);
        const std::string &varName = compiler.get_name(res.id);
        // const std::string &blockName = compiler.get_name(res.base_type_id);
        
        VkDescriptorSetLayoutBinding uboBinding = {};
        uboBinding.binding = binding;
        uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboBinding.descriptorCount = 1;
        uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboBinding.pImmutableSamplers = nullptr;
        m_SetLayoutBindings.push_back(uboBinding);
        
        VkDescriptorPoolSize poolSize;
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 1;
        m_PoolSizes.push_back(poolSize);
        
        UniformBuffer uniformBuffer;
        CreateUniformBuffer(uniformBuffer, blockSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        m_Variables.insert(std::make_pair(varName, m_UniformBuffers.size()));
        m_UniformBuffers.push_back(uniformBuffer);
    }
    
}

void Shader::UpdatePipelineLayout()
{
    DestroyPipelineLayout();
    UpdateVertPipelineLayout();
    UpdateFragPipelineLayout();
    
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    // 创建SetLayout
    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
    ZeroVulkanStruct(setLayoutCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
    setLayoutCreateInfo.bindingCount = m_SetLayoutBindings.size();
    setLayoutCreateInfo.pBindings = m_SetLayoutBindings.data();
    VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));
    
    // 创建Pool
    VkDescriptorPoolCreateInfo poolCreateInfo;
    ZeroVulkanStruct(poolCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
    poolCreateInfo.poolSizeCount = m_PoolSizes.size();
    poolCreateInfo.pPoolSizes = m_PoolSizes.data();
    poolCreateInfo.maxSets = 1;
    VERIFYVULKANRESULT(vkCreateDescriptorPool(device, &poolCreateInfo, VULKAN_CPU_ALLOCATOR,  &m_DescriptorPool));
    
    // 分配真正的Set
    VkDescriptorSetAllocateInfo setAllococateInfo;
    ZeroVulkanStruct(setAllococateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
    setAllococateInfo.descriptorPool = m_DescriptorPool;
    setAllococateInfo.descriptorSetCount = 1;
    setAllococateInfo.pSetLayouts = &m_DescriptorSetLayout;
    VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &setAllococateInfo, &m_DescriptorSet));
    
    std::vector<VkDescriptorBufferInfo> bufferInfos(m_UniformBuffers.size());
    for (int32 i = 0; i < m_UniformBuffers.size(); ++i)
    {
        bufferInfos[i].buffer = m_UniformBuffers[i].buffer;
        bufferInfos[i].offset = 0;
        bufferInfos[i].range = m_UniformBuffers[i].size;
    }
    
    std::vector<VkWriteDescriptorSet> descriptorWrites(m_UniformBuffers.size());
    for (int32 i = 0; i < descriptorWrites.size(); ++i)
    {
        ZeroVulkanStruct(descriptorWrites[i], VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        descriptorWrites[i].dstSet = m_DescriptorSet;
        descriptorWrites[i].dstBinding = m_SetLayoutBindings[i].binding;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType = m_SetLayoutBindings[i].descriptorType;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pBufferInfo = &bufferInfos[i];
    }
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, VULKAN_CPU_ALLOCATOR);
    
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    ZeroVulkanStruct(pipelineLayoutCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
    VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
    
    m_Invalid  = false;
    m_Uploaded = true;
}

void Shader::SetUniformData(const std::string& name, uint8* dataPtr, uint32 dataSize)
{
    auto it = m_Variables.find(name);
    if (it == m_Variables.end())
    {
        return;
    }
    
    int32 index = it->second;
    if (index < 0 || index >= m_UniformBuffers.size())
    {
        return;
    }
    
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    UniformBuffer& uniformBuffer = m_UniformBuffers[index];
    uint8_t *pData = nullptr;
    VERIFYVULKANRESULT(vkMapMemory(device, uniformBuffer.memory, 0, dataSize, 0, (void**)&pData));
    std::memcpy(pData, dataPtr, dataSize);
    vkUnmapMemory(device, uniformBuffer.memory);
}

