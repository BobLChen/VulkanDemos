#include "Material.h"
#include "Engine.h"
#include "Vulkan/VulkanDevice.h"
#include "Utils/Alignment.h"
#include <vector>
#include <cstring>
#include <unordered_map>

Material::Material(std::shared_ptr<Shader> shader)
	: m_Hash(0)
	, m_InvalidStateInfo(true)
	, m_Shader(shader)
{

}

Material::~Material()
{
	m_Shader = nullptr;
}