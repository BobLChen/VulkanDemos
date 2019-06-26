#include "DrawCommand.h"
#include "Engine.h"

#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanDescriptorInfo.h"
#include "Vulkan/VulkanPipeline.h"

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

void MeshDrawCommand::Prepare()
{
	std::shared_ptr<VulkanDevice> device = Engine::Get()->GetVulkanDevice();
	VulkanGfxPipeline* pipeline = device->GetPipelineStateManager().GetGfxPipeline(material->GetPipelineStateInfo(), material->GetShader(), renderable->GetVertexBuffer()->GetVertexInputStateInfo());
    
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
