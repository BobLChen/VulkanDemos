#pragma once

#include "Common/Common.h"
#include "HAL/ThreadSafeCounter.h"
#include "Utils/Crc.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanResources.h"

#include "Graphics/Material/Material.h"
#include "Graphics/Renderer/Renderable.h"

#include <memory>
#include <vector>

class MeshDrawCommand
{
public:
	MeshDrawCommand()
	{

	}

	~MeshDrawCommand()
	{

	}

	void GenerateHash();

public:

	std::shared_ptr<Material>			material;
	std::shared_ptr<Renderable>			renderable;

	uint32								firstIndex;
	uint32								numPrimitives;
	uint32								numInstances;
};

class MeshDrawListContext
{
public:
	MeshDrawListContext()
	{

	}

	virtual ~MeshDrawListContext()
	{

	}

	virtual MeshDrawCommand* AddCommand(MeshDrawCommand* command);
	
protected:

	std::vector<MeshDrawCommand*> m_DrawCommandList;
};