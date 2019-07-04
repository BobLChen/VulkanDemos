#include "DrawCommand.h"
#include "Engine.h"

#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanDescriptorInfo.h"
#include "Vulkan/VulkanPipeline.h"
#include "Vulkan/VulkanCommandBuffer.h"

#include "Utils/Crc.h"

MeshDrawCommand::MeshDrawCommand()
	: material(nullptr)
	, renderable(nullptr)
	, firstIndex(0)
	, numPrimitives(0)
	, numInstances(0)
	, hash(0)
	, ready(false)
{
	
}

MeshDrawCommand::~MeshDrawCommand()
{

}

void MeshDrawCommand::GenerateHash()
{
	uint32 tempHash = material->GetStateHash();
	hash = Crc::MemCrc32(&tempHash, sizeof(uint32), 0);
	tempHash = material->GetShader()->GetHash();
	hash = Crc::MemCrc32(&tempHash, sizeof(uint32), hash);
	tempHash = renderable->GetIndexBuffer()->GetHash();
	hash = Crc::MemCrc32(&tempHash, sizeof(uint32), hash);
	tempHash = renderable->GetVertexBuffer()->GetHash();
	hash = Crc::MemCrc32(&tempHash, sizeof(uint32), hash);
}

void MeshDrawCommand::Prepare(VulkanCmdBuffer* cmdBuffer, VulkanCommandListContext* cmdListContext)
{
    VkCommandBuffer vkCmdBuffer = cmdBuffer->GetHandle();
    
	std::shared_ptr<VulkanDevice> device = Engine::Get()->GetVulkanDevice();
	VulkanGfxPipeline* pipeline = device->GetPipelineStateManager().GetGfxPipeline(material, renderable);
    pipeline->UpdateDescriptorSets(material, cmdBuffer, cmdListContext);
	pipeline->BindDescriptorSets(vkCmdBuffer);
    
    const std::vector<VkBuffer>& vertexBuffers = renderable->GetVertexBuffer()->GetVKBuffers();
    VkBuffer indexBuffer  = renderable->GetIndexBuffer()->GetBuffer();
    VkIndexType indexType = renderable->GetIndexBuffer()->GetIndexType();
    uint32 indexCount = renderable->GetIndexBuffer()->GetIndexCount();
    
    VkDeviceSize offsets[1] = { 0 };
    vkCmdBindPipeline(vkCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetHandle());
    vkCmdBindVertexBuffers(vkCmdBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets);
    vkCmdBindIndexBuffer(vkCmdBuffer, indexBuffer, 0, indexType);
    vkCmdDrawIndexed(vkCmdBuffer, indexCount, 0, 0, 0, 0);
}

void MeshDrawCommand::Reset()
{
	material		= nullptr;
	renderable		= nullptr;

	firstIndex		= 0;
	numPrimitives	= 0;
	numInstances	= 0;

	hash	= 0;
	ready	= false;
}

MeshDrawListContext::MeshDrawListContext()
{

}

MeshDrawListContext::~MeshDrawListContext()
{

}

MeshDrawCommand* MeshDrawListContext::AddCommand(MeshDrawCommand* command)
{
	m_DrawCommandList.push_back(command);
	return command;
}
