#include "Graphics/Shader/Shader.h"

#include "VulkanDevice.h"
#include "VulkanDescriptorInfo.h"
#include "VulkanPipeline.h"
#include "Engine.h"
#include "spirv_cross.hpp"

// VulkanDescriptorSetsLayout
void VulkanDescriptorSetsLayout::Compile()
{
	m_Hash = 0;
	const int32 layoutCount = m_SetLayouts.size();
	for (int32 i = 0; i < layoutCount; ++i)
	{
		m_SetLayouts[i]->GenerateHash();
		m_Hash = Crc::MemCrc32(&(m_SetLayouts[i]->hash), sizeof(uint32), m_Hash);
	}

	if (m_LayoutHandles.size() != 0) {
		MLOGE("Layout handles generated!");
		return;
	}

	const VkPhysicalDeviceLimits& limits = Engine::Get()->GetVulkanDevice()->GetLimits();
	if (m_LayoutTypes[VK_DESCRIPTOR_TYPE_SAMPLER] + 
		m_LayoutTypes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] >= limits.maxDescriptorSetSamplers) 
	{
		MLOGE("Max DescriptorSetSamplers!");
		return;
	}

	if (m_LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] + 
		m_LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >= limits.maxDescriptorSetUniformBuffers) 
	{
		MLOGE("Max DescriptorSetUniformBuffers!");
		return;
	}

	if (m_LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >= limits.maxDescriptorSetUniformBuffersDynamic) 
	{
		MLOGE("Max DescriptorSetUniformBuffersDynamic!");
		return;
	}

	if (m_LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] +
		m_LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC] >= limits.maxDescriptorSetUniformBuffers) 
	{
		MLOGE("Max DescriptorSetUniformBuffers!");
		return;
	}

	if (m_LayoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC] >= limits.maxDescriptorSetStorageBuffersDynamic) 
	{
		MLOGE("Max DescriptorSetStorageBuffersDynamic!");
		return;
	}

	if (m_LayoutTypes[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] + 
		m_LayoutTypes[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE] + 
		m_LayoutTypes[VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER] >= limits.maxDescriptorSetSampledImages) 
	{
		MLOGE("Max DescriptorSetSampledImages!");
		return;
	}

	if (m_LayoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE] + 
		m_LayoutTypes[VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER] >= limits.maxDescriptorSetStorageImages) 
	{
		MLOGE("Max DescriptorSetStorageImages!");
		return;
	}

	if (m_LayoutTypes[VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT] >= limits.maxDescriptorSetInputAttachments) 
	{
		MLOGE("Max DescriptorSetInputAttachments!");
		return;
	}

	m_LayoutHandles.resize(m_SetLayouts.size());
	for (int32 i = 0; i < m_SetLayouts.size(); ++i)
	{
		VulkanDescriptorSetLayoutInfo* setLayoutInfo = m_SetLayouts[i];
		m_LayoutHandles[i] = Engine::Get()->GetVulkanDevice()->GetPipelineStateManager().GetDescriptorSetLayout(setLayoutInfo);
	}

	
}

void VulkanDescriptorSetsLayout::AddDescriptor(uint32 set, uint32 binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, VkSampler* samplers)
{
	m_LayoutTypes[descriptorType] += 1;

	VulkanDescriptorSetLayoutInfo* setLayout = nullptr;

	for (int32 i = 0; i < m_SetLayouts.size(); ++i)
	{
		if (m_SetLayouts[i]->set == set) {
			setLayout = m_SetLayouts[i];
			break;
		}
	}

	if (setLayout == nullptr) 
	{
		setLayout = new VulkanDescriptorSetLayoutInfo();
		m_SetLayouts.push_back(setLayout);
	}

	for (int32 i = 0; i < setLayout->layoutBindings.size(); ++i)
	{
		VkDescriptorSetLayoutBinding& layoutBinding = setLayout->layoutBindings[i];
		if (layoutBinding.binding == binding && 
			layoutBinding.descriptorType == descriptorType && 
			layoutBinding.stageFlags == stageFlags && 
			layoutBinding.pImmutableSamplers == samplers
		)
		{
			layoutBinding.descriptorCount += 1;
			return;
		}
	}

	VkDescriptorSetLayoutBinding bindingInfo = {};
	bindingInfo.descriptorCount    = 1;
	bindingInfo.descriptorType     = descriptorType;
	bindingInfo.stageFlags         = stageFlags;
	bindingInfo.pImmutableSamplers = samplers;

	setLayout->layoutBindings.push_back(bindingInfo);
}

// VulkanLayout
VulkanLayout::VulkanLayout()
	: m_PipelineLayout(VK_NULL_HANDLE)
{

}

VulkanLayout::~VulkanLayout()
{
	if (m_PipelineLayout != VK_NULL_HANDLE) {
		VkDevice device  = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		vkDestroyPipelineLayout(device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
		m_PipelineLayout = VK_NULL_HANDLE;
	}
}

void VulkanLayout::ProcessBindingsForStage(std::shared_ptr<ShaderModule> shaderModule)
{
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

void VulkanLayout::Compile()
{
	m_SetsLayoutInfo.Compile();

	const std::vector<VkDescriptorSetLayout>& layoutHandles = m_SetsLayoutInfo.GetHandles();

	VkPipelineLayoutCreateInfo pipeLayoutInfo;
	ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
	pipeLayoutInfo.setLayoutCount = layoutHandles.size();
	pipeLayoutInfo.pSetLayouts    = layoutHandles.data();
	VERIFYVULKANRESULT(vkCreatePipelineLayout(Engine::Get()->GetDeviceHandle(), &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
}

// VulkanGfxLayout
VulkanGfxLayout::VulkanGfxLayout()
{

}

VulkanGfxLayout::~VulkanGfxLayout()
{

}

// VulkanComputeLayout
VulkanComputeLayout::VulkanComputeLayout()
{

}

VulkanComputeLayout::~VulkanComputeLayout()
{

}

// VulkanDescriptorPool
VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice* inDevice, const VulkanDescriptorSetsLayout& layout, uint32 maxSetsAllocations)
	: m_Device(inDevice)
	, m_MaxDescriptorSets(maxSetsAllocations)
	, m_NumAllocatedDescriptorSets(0)
	, m_PeakAllocatedDescriptorSets(0)
	, m_DescriptorPool(VK_NULL_HANDLE)
	, m_Layout(layout)
{
	std::vector<VkDescriptorPoolSize> types;
	for (uint32 typeIndex = VK_DESCRIPTOR_TYPE_BEGIN_RANGE; typeIndex <= VK_DESCRIPTOR_TYPE_END_RANGE; ++typeIndex)
	{
		VkDescriptorType descriptorType =(VkDescriptorType)typeIndex;
		uint32 numTypesUsed = layout.GetTypesUsed(descriptorType);
		if (numTypesUsed > 0)
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.descriptorCount = numTypesUsed * m_MaxDescriptorSets;
			poolSize.type = descriptorType;
			types.push_back(poolSize);
		}
	}

	VkDescriptorPoolCreateInfo poolCreateInfo;
	ZeroVulkanStruct(poolCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
	poolCreateInfo.poolSizeCount = types.size();
	poolCreateInfo.pPoolSizes    = types.data();
	poolCreateInfo.maxSets       = m_MaxDescriptorSets;
	vkCreateDescriptorPool(inDevice->GetInstanceHandle(), &poolCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool);
}

VulkanDescriptorPool::~VulkanDescriptorPool()
{
	if (m_DescriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(m_Device->GetInstanceHandle(), m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
		m_DescriptorPool = VK_NULL_HANDLE;
	}
}

void VulkanDescriptorPool::TrackAddUsage(const VulkanDescriptorSetsLayout& inLayout)
{
	if (m_Layout.GetHash() != inLayout.GetHash())
	{
		MLOGE("VulkanDescriptorSetsLayout not equal.");
		return;
	}

	m_NumAllocatedDescriptorSets += inLayout.GetLayouts().size();
	m_PeakAllocatedDescriptorSets = MMath::Max(m_NumAllocatedDescriptorSets, m_PeakAllocatedDescriptorSets);
}
    
void VulkanDescriptorPool::TrackRemoveUsage(const VulkanDescriptorSetsLayout& inLayout)
{
	if (m_Layout.GetHash() != inLayout.GetHash())
	{
		MLOGE("VulkanDescriptorSetsLayout not equal.");
		return;
	}

	m_NumAllocatedDescriptorSets -= inLayout.GetLayouts().size();
}
    
void VulkanDescriptorPool::Reset()
{
	if (m_DescriptorPool != VK_NULL_HANDLE)
	{
		vkResetDescriptorPool(m_Device->GetInstanceHandle(), m_DescriptorPool, 0);
	}

	m_NumAllocatedDescriptorSets = 0;
}
    
bool VulkanDescriptorPool::AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& inDescriptorSetAllocateInfo, VkDescriptorSet* outSets)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = inDescriptorSetAllocateInfo;
	descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;

	return vkAllocateDescriptorSets(m_Device->GetInstanceHandle(), &descriptorSetAllocateInfo, outSets);
}

// VulkanTypedDescriptorPoolSet
VulkanTypedDescriptorPoolSet::VulkanTypedDescriptorPoolSet(VulkanDevice* inDevice, const VulkanDescriptorSetsLayout& inLayout)
	: m_Device(inDevice)
	, m_PoolsCount(0)
	, m_CurrentPool(0)
	, m_Layout(inLayout)
{
	PushNewPool();
}

VulkanTypedDescriptorPoolSet::~VulkanTypedDescriptorPoolSet()
{
	for (int32 i = 0; i < m_PoolsList.size(); ++i)
	{
		delete m_PoolsList[i];
	}
	m_PoolsList.clear();
	m_PoolsCount = 0;
}

bool VulkanTypedDescriptorPoolSet::AllocateDescriptorSets(const VulkanDescriptorSetsLayout& inLayout, VkDescriptorSet* outSets)
{
	const std::vector<VkDescriptorSetLayout>& layoutHandles = inLayout.GetHandles();

	if (layoutHandles.size() > 0)
	{
		while (m_PoolsList[m_PoolsCount]->AllocateDescriptorSets(inLayout.GetAllocateInfo(), outSets))
		{
			GetFreePool(true);
		}
		m_PoolsList[m_PoolsCount]->TrackAddUsage(inLayout);
	}

	return true;
}

void VulkanTypedDescriptorPoolSet::Reset()
{
	for (int32 i = 0; i < m_PoolsList.size(); ++i)
	{
		m_PoolsList[i]->Reset();
	}
	m_CurrentPool = 0;
}

VulkanDescriptorPool* VulkanTypedDescriptorPoolSet::GetFreePool(bool forceNewPool = false)
{
	if (!forceNewPool)
	{
		return m_PoolsList[m_CurrentPool];
	}

	if (m_CurrentPool + 1 < m_PoolsList.size())
	{
		m_CurrentPool = m_CurrentPool + 1;
		return m_PoolsList[m_CurrentPool];
	}

	return PushNewPool();
}

VulkanDescriptorPool* VulkanTypedDescriptorPoolSet::PushNewPool()
{
	const uint32 MaxSetsAllocations = 32 << MMath::Min(m_PoolsCount++, 2u);
	VulkanDescriptorPool* newPool = new VulkanDescriptorPool(m_Device, m_Layout, MaxSetsAllocations);
	m_PoolsList.push_back(newPool);
	m_CurrentPool = m_PoolsList.size() - 1;
	return newPool;
}