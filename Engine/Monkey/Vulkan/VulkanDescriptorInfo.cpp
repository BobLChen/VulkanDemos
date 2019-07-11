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
		m_SetLayouts[i]->Compile();
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

    ZeroVulkanStruct(m_DescriptorSetAllocateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
    m_DescriptorSetAllocateInfo.descriptorSetCount = m_LayoutHandles.size();
    m_DescriptorSetAllocateInfo.pSetLayouts        = m_LayoutHandles.data();
}

int32 VulkanDescriptorSetsLayout::AddDescriptor(uint32 set, uint32 binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, VkSampler* samplers)
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
        setLayout->set = set;
		m_SetLayouts.push_back(setLayout);
	}
    
	VkDescriptorSetLayoutBinding bindingInfo = {};
	bindingInfo.descriptorCount    = 1;
	bindingInfo.descriptorType     = descriptorType;
	bindingInfo.stageFlags         = stageFlags;
	bindingInfo.pImmutableSamplers = samplers;

	setLayout->layoutBindings.push_back(bindingInfo);

	return setLayout->layoutBindings.size() - 1;
}

// VulkanLayout
VulkanLayout::VulkanLayout(const VulkanDescriptorSetsLayout& inSetsLayout)
	: m_PipelineLayout(VK_NULL_HANDLE)
	, m_SetsLayout(inSetsLayout)
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

void VulkanLayout::Compile()
{
	const std::vector<VkDescriptorSetLayout>& layoutHandles = m_SetsLayout.GetHandles();

	VkPipelineLayoutCreateInfo pipeLayoutInfo;
	ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
	pipeLayoutInfo.setLayoutCount = layoutHandles.size();
	pipeLayoutInfo.pSetLayouts    = layoutHandles.data();
	VERIFYVULKANRESULT(vkCreatePipelineLayout(Engine::Get()->GetDeviceHandle(), &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
}

// VulkanGfxLayout
VulkanGfxLayout::VulkanGfxLayout(const VulkanDescriptorSetsLayout& inSetsLayout)
	: VulkanLayout(inSetsLayout)
{

}

VulkanGfxLayout::~VulkanGfxLayout()
{

}

// VulkanComputeLayout
VulkanComputeLayout::VulkanComputeLayout(const VulkanDescriptorSetsLayout& inSetsLayout)
	: VulkanLayout(inSetsLayout)
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
	if (m_DescriptorPool != VK_NULL_HANDLE) {
		vkResetDescriptorPool(m_Device->GetInstanceHandle(), m_DescriptorPool, 0);
	}

	m_NumAllocatedDescriptorSets = 0;
}
    
bool VulkanDescriptorPool::AllocateDescriptorSets(const VkDescriptorSetAllocateInfo& inDescriptorSetAllocateInfo, VkDescriptorSet* outSets)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = inDescriptorSetAllocateInfo;
	descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;

	return VK_SUCCESS == vkAllocateDescriptorSets(m_Device->GetInstanceHandle(), &descriptorSetAllocateInfo, outSets);
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
	for (int32 i = 0; i < m_PoolsList.size(); ++i) {
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
		while (!(m_PoolsList[m_CurrentPool]->AllocateDescriptorSets(inLayout.GetAllocateInfo(), outSets))) {
			GetFreePool(true);
		}
		m_PoolsList[m_CurrentPool]->TrackAddUsage(inLayout);
	}

	return true;
}

void VulkanTypedDescriptorPoolSet::Reset()
{
	for (int32 i = 0; i < m_PoolsList.size(); ++i) {
		m_PoolsList[i]->Reset();
	}
	m_CurrentPool = 0;
}

VulkanDescriptorPool* VulkanTypedDescriptorPoolSet::GetFreePool(bool forceNewPool)
{
	if (!forceNewPool) {
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

// VulkanDescriptorPoolSetContainer
VulkanDescriptorPoolSetContainer::~VulkanDescriptorPoolSetContainer()
{
    for (auto it = m_TypedDescriptorPools.begin(); it != m_TypedDescriptorPools.end(); ++it)
    {
        VulkanTypedDescriptorPoolSet* poolSet = it->second;
        delete poolSet;
    }
    m_TypedDescriptorPools.clear();
}

VulkanTypedDescriptorPoolSet* VulkanDescriptorPoolSetContainer::AcquireTypedPoolSet(const VulkanDescriptorSetsLayout& layout)
{
    const uint32 hash = layout.GetHash();
    auto it = m_TypedDescriptorPools.find(hash);
    VulkanTypedDescriptorPoolSet* poolSet = nullptr;
    
    if (it == m_TypedDescriptorPools.end())
    {
        poolSet = new VulkanTypedDescriptorPoolSet(m_VulkanDevice, layout);
        m_TypedDescriptorPools.insert(std::make_pair(hash, poolSet));
    }
    else
    {
        poolSet = it->second;
    }
    
    return poolSet;
}

void VulkanDescriptorPoolSetContainer::Reset()
{
    for (auto it = m_TypedDescriptorPools.begin(); it != m_TypedDescriptorPools.end(); ++it)
    {
        VulkanTypedDescriptorPoolSet* poolSet = it->second;
        poolSet->Reset();
    }
}

// VulkanDescriptorPoolsManager

VulkanDescriptorPoolSetContainer& VulkanDescriptorPoolsManager::AcquirePoolSetContainer()
{
	for (int32 i = 0; i < m_PoolSets.size(); ++i)
	{
		if (m_PoolSets[i]->IsUnused())
		{
			m_PoolSets[i]->SetUsed(true);
			return *(m_PoolSets[i]);
		}
	}

	VulkanDescriptorPoolSetContainer* poolSet = new VulkanDescriptorPoolSetContainer(m_VulkanDevice);
	m_PoolSets.push_back(poolSet);

	return *poolSet;
}
    
void VulkanDescriptorPoolsManager::ReleasePoolSet(VulkanDescriptorPoolSetContainer& poolSet)
{
	poolSet.Reset();
	poolSet.SetUsed(false);
}
    
void VulkanDescriptorPoolsManager::GC()
{
	for (int32 i = m_PoolSets.size() - 1; i >= 0; --i)
	{
		VulkanDescriptorPoolSetContainer* poolSet = m_PoolSets[i];
		if (poolSet->IsUnused())
		{
			m_PoolSets.erase(m_PoolSets.begin() + i);
			delete poolSet;
		}
	}
}
    
void VulkanDescriptorPoolsManager::Destroy()
{
	for (int32 i = 0; i < m_PoolSets.size(); ++i) {
		delete m_PoolSets[i];
	}
	m_PoolSets.clear();
}

// VulkanDescriptorSetWriter
uint32 VulkanDescriptorSetWriter::SetupDescriptorWrites(const std::vector<VkDescriptorSetLayoutBinding>& bindings, VkWriteDescriptorSet* inWriteDescriptors, VkDescriptorImageInfo* inImageInfo, VkDescriptorBufferInfo* inBufferInfo, uint8* inBindingToDynamicOffsetMap)
{
    m_NumWrites = bindings.size();
	m_WriteDescriptorSet = inWriteDescriptors;
	m_BindingToDynamicOffsetMap = inBindingToDynamicOffsetMap;
    
	int32 dynamicOffsetIndex = 0;
	for (int32 i = 0; i < m_NumWrites; ++i)
	{
        const VkDescriptorSetLayoutBinding& setBinding = bindings[i];
        
		inWriteDescriptors->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        inWriteDescriptors->pNext = nullptr;
        inWriteDescriptors->dstSet = VK_NULL_HANDLE;
        inWriteDescriptors->dstBinding = setBinding.binding;
        inWriteDescriptors->dstArrayElement = 0;
		inWriteDescriptors->descriptorCount = setBinding.descriptorCount;
		inWriteDescriptors->descriptorType = setBinding.descriptorType;
        inWriteDescriptors->pImageInfo = nullptr;
        inWriteDescriptors->pBufferInfo = nullptr;
        inWriteDescriptors->pTexelBufferView = nullptr;
        
		switch (setBinding.descriptorType)
		{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				m_BindingToDynamicOffsetMap[i] = dynamicOffsetIndex++;
				inWriteDescriptors->pBufferInfo = inBufferInfo++;
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
				inWriteDescriptors->pBufferInfo = inBufferInfo++;
				break;
			case VK_DESCRIPTOR_TYPE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				// image info
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
				break;
			default:
				MLOGE("Unsupported descriptor type %d", (int32)setBinding.descriptorType);
				break;
		}

		++inWriteDescriptors;
	}
    
	return dynamicOffsetIndex;
}
