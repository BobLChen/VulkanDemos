#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanRHI.h"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

class ShaderModule
{
public:
	ShaderModule(VkShaderModule shaderModule)
		: m_ShaderModule(shaderModule)
	{

	}

	const VkShaderModule& GetHandle() const
	{
		return m_ShaderModule;
	}

	virtual ~ShaderModule();

protected:
	VkShaderModule m_ShaderModule;
};

class Shader
{
public:

	Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom = nullptr, std::shared_ptr<ShaderModule> comp = nullptr, std::shared_ptr<ShaderModule> tesc = nullptr, std::shared_ptr<ShaderModule> tese = nullptr);

	virtual ~Shader();

	static std::shared_ptr<Shader> Create(const char* vert, const char* frag, const char* geometry = nullptr, const char* compute = nullptr, const char* tessControl = nullptr, const char* tessEvaluate = nullptr);

	static std::shared_ptr<ShaderModule> LoadSPIPVShader(const std::string& filename);

	FORCEINLINE VkPipelineLayout GetPipelineLayout()
	{
		if (m_Invalid)
		{
			UpdatePipelineLayout();
		}
		return m_PipelineLayout;
	}

	FORCEINLINE const std::vector<VkPipelineShaderStageCreateInfo>& GetStages()
	{
		if (m_Invalid)
		{
			UpdatePipelineLayout();
		}
		return m_Stages;
	}

protected:

	void UpdatePipelineLayout();
	
	static std::unordered_map<std::string, std::shared_ptr<ShaderModule>> g_ShaderModules;

	VkPipelineLayout m_PipelineLayout;

	std::shared_ptr<ShaderModule> m_VertShaderModule;
	std::shared_ptr<ShaderModule> m_FragShaderModule;
	std::shared_ptr<ShaderModule> m_GeomShaderModule;
	std::shared_ptr<ShaderModule> m_CompShaderModule;
	std::shared_ptr<ShaderModule> m_TescShaderModule;
	std::shared_ptr<ShaderModule> m_TeseShaderModule;

	bool m_Invalid;
	std::vector<VkPipelineShaderStageCreateInfo> m_Stages;
};
