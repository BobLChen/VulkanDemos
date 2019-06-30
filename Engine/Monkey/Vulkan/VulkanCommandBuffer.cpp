#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanMemory.h"
#include "VulkanFence.h"

// VulkanCommandBufferPool
VulkanCommandBufferPool::VulkanCommandBufferPool(VulkanDevice* inDevice, VulkanCommandBufferManager& inManager)
	: m_Handle(VK_NULL_HANDLE)
	, m_Device(nullptr)
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

void VulkanCommandBufferPool::FreeUnusedCmdBuffers(VulkanQueue* queue)
{

}

VulkanCmdBuffer* VulkanCommandBufferPool::Create(bool isUploadOnly)
{

}

void VulkanCommandBufferPool::Create(uint32 queueFamilyIndex)
{

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

	m_ActiveCmdBuffer = m_CmdBufferPool.Create(false);
	m_ActiveCmdBuffer->Begin();
}

void VulkanCommandBufferManager::FreeUnusedCmdBuffers()
{
	m_CmdBufferPool->FreeUnusedCmdBuffers(m_Queue);
}