#include "Material.h"
#include "Engine.h"

#include "Vulkan/VulkanDevice.h"

#include "Utils/Alignment.h"
#include "Graphics/Shader/Shader.h"

#include <vector>
#include <cstring>
#include <unordered_map>

Material::Material(std::shared_ptr<Shader> shader)
	: m_Hash(0)
	, m_InvalidStateInfo(true)
	, m_Shader(nullptr)
{
	SetShader(shader);
}

Material::~Material()
{
	m_Shader = nullptr;
}

void Material::SetParam(const std::string& name, const void* data, uint32 size)
{
	for (int32 i = 0; i < m_UniformBufferParams.size(); ++i)
	{
		UniformBufferParam& ubParam = m_UniformBufferParams[i];
		if (ubParam.name == name) {
			ubParam.buffer->UpdateConstantData(data, size);
			return;
		}
	}
}

void Material::GenerateShaderParams()
{
	const std::vector<ShaderParamInfo> params = m_Shader->GetParams();

	for (int32 i = 0; i < params.size(); ++i)
	{
		const ShaderParamInfo& param = params[i];

		switch (param.descriptorType)
		{
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				{
					UniformBufferParam uboParam = {};
					uboParam.name	 = param.name;
					uboParam.set     = param.set;
					uboParam.binding = param.binding;
					uboParam.buffer  = new VulkanUniformBuffer(param.bufferSize);
					uboParam.descriptorIndex = param.descriptorIndex;
					m_UniformBufferParams.push_back(uboParam);
				}
				break;
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:

				break;
			case VK_DESCRIPTOR_TYPE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
			case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
			case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
				// image
				break;
			case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
				break;
			default:
				MLOGE("Unsupported descriptor type %d", (int32)param.descriptorType);
				break;
		}

	}
}