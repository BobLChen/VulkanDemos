#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Graphics/Renderer/Renderable.h"

#include <memory>
#include <string>
#include <vector>

class OBJMeshParser
{
public:

	static std::vector<std::shared_ptr<Renderable>> LoadFromFile(const std::string& filename);

};