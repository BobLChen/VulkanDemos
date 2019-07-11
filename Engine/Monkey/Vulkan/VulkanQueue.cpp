#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanFence.h"
#include "VulkanCommandBuffer.h"

VulkanQueue::VulkanQueue(VulkanDevice* device, uint32 familyIndex)
    : m_Queue(VK_NULL_HANDLE)
    , m_FamilyIndex(familyIndex)
    , m_Device(device)
{
    vkGetDeviceQueue(m_Device->GetInstanceHandle(), m_FamilyIndex, 0, &m_Queue);
}

VulkanQueue::~VulkanQueue()
{
    
}

void VulkanQueue::Submit(VulkanCmdBuffer* cmdBuffer, uint32 numSignalSemaphores, VkSemaphore* signalSemaphores)
{
    VulkanFence* fence = cmdBuffer->GetFence();
    const VkCommandBuffer cmdBuffers[] = { cmdBuffer->GetHandle() };
    
    VkSubmitInfo submitInfo;
    ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = cmdBuffers;
    submitInfo.signalSemaphoreCount = numSignalSemaphores;
    submitInfo.pSignalSemaphores    = signalSemaphores;
    
    std::vector<VkSemaphore> waitSemaphores;
    if (cmdBuffer->GetWaitSemaphores().size() > 0)
    {
        waitSemaphores.resize(cmdBuffer->GetWaitSemaphores().size());
        for (int32 i = 0; i < cmdBuffer->GetWaitSemaphores().size(); ++i) {
			waitSemaphores[i] = cmdBuffer->GetWaitSemaphores()[i];
        }

        submitInfo.waitSemaphoreCount = waitSemaphores.size();
        submitInfo.pWaitSemaphores    = waitSemaphores.data();
        submitInfo.pWaitDstStageMask  = cmdBuffer->GetWaitFlags().data();
    }

    vkQueueSubmit(m_Queue, 1, &submitInfo, fence->GetHandle());
    
    cmdBuffer->SetState(VulkanCmdBuffer::State::Submitted);
    cmdBuffer->MarkSemaphoresAsSubmitted();
    cmdBuffer->RefreshSubmittedFenceCounter();
    
    m_Device->GetFenceManager().WaitForFence(cmdBuffer->GetFence(), 200 * 1000 * 1000);
    cmdBuffer->GetOwner()->RefreshFenceStatus(cmdBuffer);
}
