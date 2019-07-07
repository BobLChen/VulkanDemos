#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanMemory.h"
#include "VulkanFence.h"
#include "VulkanResources.h"
#include "VulkanDescriptorInfo.h"
#include "VulkanPipeline.h"

// VulkanCmdBuffer
VulkanCmdBuffer::VulkanCmdBuffer(VulkanDevice* inDevice, VulkanCommandBufferPool* inCommandBufferPool, bool inIsUploadOnly)
	: m_StencilRef(0)
	, m_State(State::NotAllocated)
	, m_IsUploadOnly(inIsUploadOnly)
	, m_FenceSignaledCounter(0)
	, m_SubmittedFenceCounter(0)
	, m_CommandBufferPool(inCommandBufferPool)
	, m_VulkanDevice(inDevice)
	, m_Handle(VK_NULL_HANDLE)
	, m_Fence(nullptr)
	, m_DescriptorPoolSetContainer(nullptr)
{
	AllocMemory();
	m_Fence = inDevice->GetFenceManager().CreateFence();
}

VulkanCmdBuffer::~VulkanCmdBuffer()
{
	VulkanFenceManager& fenceManager = m_VulkanDevice->GetFenceManager();
	if (m_State == State::Submitted)
	{
		// 33ms
		uint64 waitForCmdBufferInNanoSeconds = 33 * 1000 * 1000LL;
		fenceManager.WaitAndReleaseFence(m_Fence, waitForCmdBufferInNanoSeconds);
	}
	else 
	{
		fenceManager.ReleaseFence(m_Fence);
	}

	if (m_State != State::NotAllocated)
	{
		FreeMemory();
	}
}

void VulkanCmdBuffer::MarkSemaphoresAsSubmitted()
{
    m_WaitFlags.clear();
    m_SubmittedWaitSemaphores = m_WaitSemaphores;
    m_WaitSemaphores.clear();
}

void VulkanCmdBuffer::RefreshSubmittedFenceCounter()
{
    m_SubmittedFenceCounter = m_FenceSignaledCounter;
}

void VulkanCmdBuffer::AddWaitSemaphore(VkPipelineStageFlags inWaitFlags, VkSemaphore inWaitSemaphore)
{
	m_WaitFlags.push_back(inWaitFlags);
	m_WaitSemaphores.push_back(inWaitSemaphore);
}

void VulkanCmdBuffer::Begin()
{
	if (m_State != State::ReadyForBegin) {
		MLOGE("State not ready for begin.");
		return;
	}

	m_State = State::IsInsideBegin;

	VkCommandBufferBeginInfo cmdBufBeginInfo;
	ZeroVulkanStruct(cmdBufBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
	cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(m_Handle, &cmdBufBeginInfo);
}

void VulkanCmdBuffer::End()
{
	vkEndCommandBuffer(m_Handle);
	m_State = State::HasEnded;
}

bool VulkanCmdBuffer::AcquirePoolSetAndDescriptorsIfNeeded(const class VulkanDescriptorSetsLayout& layout, VkDescriptorSet* outDescriptors)
{
	if (!m_DescriptorPoolSetContainer) {
		AcquirePoolSetContainer();
	}

	const uint32 hash = layout.GetHash();

	auto it = m_TypedDescriptorPoolSets.find(hash);
	if (it != m_TypedDescriptorPoolSets.end()) {
		return false;
	}

	VulkanTypedDescriptorPoolSet* poolSet = m_DescriptorPoolSetContainer->AcquireTypedPoolSet(layout);
	return poolSet->AllocateDescriptorSets(layout, outDescriptors);
}

void VulkanCmdBuffer::RefreshFenceStatus()
{
	if (m_State == State::Submitted)
	{
		VulkanFenceManager* fenceManager = m_Fence->GetOwner();
		if (fenceManager->IsFenceSignaled(m_Fence))
		{
			m_SubmittedWaitSemaphores.clear();

			memset(&m_Viewport, 0, sizeof(m_Viewport));
			memset(&m_Scissor,  0, sizeof(m_Scissor));
			m_StencilRef = 0;

			vkResetCommandBuffer(m_Handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
			m_Fence->GetOwner()->ResetFence(m_Fence);

			m_FenceSignaledCounter += 1;

			if (m_DescriptorPoolSetContainer)
			{
				m_TypedDescriptorPoolSets.clear();
				m_VulkanDevice->GetDescriptorPoolsManager().ReleasePoolSet(*m_DescriptorPoolSetContainer);
				m_DescriptorPoolSetContainer = nullptr;
			}
			
			m_State = State::ReadyForBegin;
		}
	}
}

void VulkanCmdBuffer::AcquirePoolSetContainer()
{
	m_DescriptorPoolSetContainer = &(m_VulkanDevice->GetDescriptorPoolsManager().AcquirePoolSetContainer());
}

void VulkanCmdBuffer::AllocMemory()
{
	memset(&m_Viewport, 0, sizeof(m_Viewport));
	memset(&m_Scissor,  0, sizeof(m_Scissor));

	VkCommandBufferAllocateInfo cmdBufferInfo;
	ZeroVulkanStruct(cmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
	cmdBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferInfo.commandBufferCount = 1;
	cmdBufferInfo.commandPool        = m_CommandBufferPool->GetHandle();
	vkAllocateCommandBuffers(m_VulkanDevice->GetInstanceHandle(), &cmdBufferInfo, &m_Handle);

	m_State = State::ReadyForBegin;
}

void VulkanCmdBuffer::FreeMemory()
{
	if (m_State == State::NotAllocated) {
		return;
	}
	
	if (m_Handle == VK_NULL_HANDLE) {
		return;
	}

	vkFreeCommandBuffers(m_VulkanDevice->GetInstanceHandle(), m_CommandBufferPool->GetHandle(), 1, &m_Handle);
	m_Handle = VK_NULL_HANDLE;
	m_State  = State::NotAllocated;
}

// VulkanCommandBufferPool
VulkanCommandBufferPool::VulkanCommandBufferPool(VulkanDevice* inDevice, VulkanCommandBufferManager& inManager)
	: m_Handle(VK_NULL_HANDLE)
	, m_Device(inDevice)
	, m_CmdBufferManager(inManager)
{
	
}

VulkanCommandBufferPool::~VulkanCommandBufferPool()
{
	for (int32 i = 0; i < m_UsedCmdBuffers.size(); ++i)
	{
		VulkanCmdBuffer* cmdBuffer = m_UsedCmdBuffers[i];
		cmdBuffer->FreeMemory();
		delete cmdBuffer;
	}
	m_UsedCmdBuffers.clear();

	for (int32 i = 0; i < m_FreeCmdBuffers.size(); ++i)
	{
		VulkanCmdBuffer* cmdBuffer = m_FreeCmdBuffers[i];
		delete cmdBuffer;
	}
	m_FreeCmdBuffers.clear();
	
	vkDestroyCommandPool(m_Device->GetInstanceHandle(), m_Handle, VULKAN_CPU_ALLOCATOR);
	m_Handle = VK_NULL_HANDLE;
}

void VulkanCommandBufferPool::RefreshFenceStatus(VulkanCmdBuffer* skipCmdBuffer)
{
	for (int32 i = 0; i < m_UsedCmdBuffers.size(); ++i)
	{
		VulkanCmdBuffer* cmdBuffer = m_UsedCmdBuffers[i];
		if (cmdBuffer != skipCmdBuffer)
		{
			cmdBuffer->RefreshFenceStatus();
		}
	}
}

void VulkanCommandBufferPool::FreeUnusedCmdBuffers(std::shared_ptr<VulkanQueue> queue)
{
	for (int32 i = m_UsedCmdBuffers.size() - 1; i >= 0; --i)
	{
		VulkanCmdBuffer* cmdBuffer = m_UsedCmdBuffers[i];
		if (cmdBuffer->GetState() == VulkanCmdBuffer::State::ReadyForBegin)
		{
			cmdBuffer->FreeMemory();
			m_FreeCmdBuffers.push_back(cmdBuffer);
			m_UsedCmdBuffers.erase(m_UsedCmdBuffers.begin() + i);
		}
	}
}

VulkanCmdBuffer* VulkanCommandBufferPool::Create(bool isUploadOnly)
{
	for (int32 i = m_FreeCmdBuffers.size() - 1; i >= 0; --i)
	{
		VulkanCmdBuffer* cmdBuffer = m_FreeCmdBuffers[i];
		if (cmdBuffer->m_IsUploadOnly == isUploadOnly)
		{
			m_FreeCmdBuffers.erase(m_FreeCmdBuffers.begin() + i);
			m_UsedCmdBuffers.push_back(cmdBuffer);
			cmdBuffer->AllocMemory();
			return cmdBuffer;
		}
	}

	VulkanCmdBuffer* cmdBuffer = new VulkanCmdBuffer(m_Device, this, isUploadOnly);
	m_UsedCmdBuffers.push_back(cmdBuffer);
	return cmdBuffer;
}

void VulkanCommandBufferPool::Create(uint32 queueFamilyIndex)
{
	VkCommandPoolCreateInfo cmdPoolCreateInfo;
	ZeroVulkanStruct(cmdPoolCreateInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
	cmdPoolCreateInfo.queueFamilyIndex =  queueFamilyIndex;
	cmdPoolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VERIFYVULKANRESULT(vkCreateCommandPool(m_Device->GetInstanceHandle(), &cmdPoolCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Handle));
}

// VulkanCommandBufferManager
VulkanCommandBufferManager::VulkanCommandBufferManager(VulkanDevice* inDevice, VulkanCommandListContext* inContext)
	: m_Device(inDevice)
	, m_CmdBufferPool(nullptr)
	, m_Queue(inContext->GetQueue())
	, m_ActiveCmdBuffer(nullptr)
	, m_UploadCmdBuffer(nullptr)
{
	m_CmdBufferPool = new VulkanCommandBufferPool(m_Device, *this);
	m_CmdBufferPool->Create(m_Queue->GetFamilyIndex());

	m_ActiveCmdBuffer = m_CmdBufferPool->Create(false);
	m_ActiveCmdBuffer->Begin();
}

VulkanCommandBufferManager::~VulkanCommandBufferManager()
{
	delete m_CmdBufferPool;
}

VulkanCmdBuffer* VulkanCommandBufferManager::GetUploadCmdBuffer()
{
	if (!m_UploadCmdBuffer)
	{
		for (int32 i = 0; i < m_CmdBufferPool->m_UsedCmdBuffers.size(); ++i)
		{
			VulkanCmdBuffer* cmdBuffer = m_CmdBufferPool->m_UsedCmdBuffers[i];
			cmdBuffer->RefreshFenceStatus();
			if (cmdBuffer->m_IsUploadOnly)
			{
				if (cmdBuffer->m_State == VulkanCmdBuffer::State::ReadyForBegin)
				{
					m_UploadCmdBuffer = cmdBuffer;
					m_UploadCmdBuffer->Begin();
					return m_UploadCmdBuffer;
				}
			}
		}

		m_UploadCmdBuffer = m_CmdBufferPool->Create(true);
		m_UploadCmdBuffer->Begin();
	}

	return m_UploadCmdBuffer;
}

void VulkanCommandBufferManager::SubmitUploadCmdBuffer(uint32 numSignalSemaphores, VkSemaphore* signalSemaphores)
{
	if (!m_UploadCmdBuffer->IsSubmitted() && m_UploadCmdBuffer->HasBegun())
	{
		m_UploadCmdBuffer->End();
		m_Queue->Submit(m_UploadCmdBuffer, numSignalSemaphores, signalSemaphores);
	}
	m_UploadCmdBuffer = nullptr;
}

void VulkanCommandBufferManager::SubmitActiveCmdBuffer(VulkanSemaphore* signalSemaphore)
{
	if (!m_ActiveCmdBuffer->IsSubmitted() && m_ActiveCmdBuffer->HasBegun())
	{
		m_ActiveCmdBuffer->End();
		if (signalSemaphore) 
		{
			m_Queue->Submit(m_ActiveCmdBuffer, signalSemaphore->GetHandle());
		}
		else 
		{
			m_Queue->Submit(m_ActiveCmdBuffer);
		}
	}
	m_ActiveCmdBuffer = nullptr;
}

void VulkanCommandBufferManager::WaitForCmdBuffer(VulkanCmdBuffer* cmdBuffer, float timeInSecondsToWait)
{
	bool success = m_Device->GetFenceManager().WaitForFence(cmdBuffer->GetFence(), (uint64)(timeInSecondsToWait * 1e9));
	cmdBuffer->RefreshFenceStatus();
}

void VulkanCommandBufferManager::NewActiveCommandBuffer()
{
	for (int32 i = 0; i < m_CmdBufferPool->m_UsedCmdBuffers.size(); ++i)
	{
		VulkanCmdBuffer* cmdBuffer = m_CmdBufferPool->m_UsedCmdBuffers[i];
		cmdBuffer->RefreshFenceStatus();
		if (!cmdBuffer->m_IsUploadOnly)
		{
			if (cmdBuffer->m_State == VulkanCmdBuffer::State::ReadyForBegin)
			{
				m_ActiveCmdBuffer = cmdBuffer;
				m_ActiveCmdBuffer->Begin();
				return;
			}
		}
	}

	m_ActiveCmdBuffer = m_CmdBufferPool->Create(false);
	m_ActiveCmdBuffer->Begin();
}

void VulkanCommandBufferManager::FreeUnusedCmdBuffers()
{
	m_CmdBufferPool->FreeUnusedCmdBuffers(m_Queue);
}
