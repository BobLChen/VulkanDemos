#pragma once

#include "Common/Common.h"
#include "Common/Log.h"

#include "Utils/Crc.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanMemory.h"
#include "Vulkan/VulkanDescriptorInfo.h"
#include "Vulkan/VulkanResources.h"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

struct ShaderParamInfo
{
	std::string			name;
	uint32				set;
	uint32				binding;
	uint32				bufferSize;
	VkDescriptorType	descriptorType;
	VkShaderStageFlags	stageFlags;
	int32				descriptorIndex;
};

class ShaderModule
{
public:
	ShaderModule(VkShaderStageFlagBits stageFlasg, VkShaderModule shaderModule, uint32* dataPtr, uint32 dataSize)
		: m_StageFlags(stageFlasg)
		, m_ShaderModule(shaderModule)
        , m_Data(dataPtr)
        , m_DataSize(dataSize)
        , m_Hash(0)
	{
        m_Hash = Crc::MemCrc32(dataPtr, dataSize);
	}
    
	inline const VkShaderModule& GetHandle() const
	{
		return m_ShaderModule;
	}
    
    inline const uint32* GetData() const
    {
        return m_Data;
    }
    
    inline const uint32 GetDataSize() const
    {
        return m_DataSize;
    }
    
    inline const uint32 GetHash() const
    {
        return m_Hash;
    }

	inline const VkShaderStageFlagBits GetStageFlags() const
	{
		return m_StageFlags;
	}

	virtual ~ShaderModule();
    
protected:

	VkShaderStageFlagBits	m_StageFlags;
	VkShaderModule			m_ShaderModule;
    uint32*					m_Data;
    uint32					m_DataSize;
    uint32					m_Hash;
};

class Shader
{

public:

	Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom = nullptr, std::shared_ptr<ShaderModule> comp = nullptr, std::shared_ptr<ShaderModule> tesc = nullptr, std::shared_ptr<ShaderModule> tese = nullptr);

	virtual ~Shader();

	static std::shared_ptr<Shader> Create(const char* vert, const char* frag, const char* geom = nullptr, const char* comp = nullptr, const char* tesc = nullptr, const char* tese = nullptr);

	static std::shared_ptr<ShaderModule> LoadSPIPVShader(const std::string& filename, VkShaderStageFlagBits stageFlags);

	static std::shared_ptr<ShaderModule> LoadSPIPVShader(const char* filename, VkShaderStageFlagBits stageFlags);
    
    inline const std::shared_ptr<ShaderModule> GetVertModule() const
    {
        return m_VertShaderModule;
    }
    
    inline const std::shared_ptr<ShaderModule> GetFragModule() const
    {
        return m_FragShaderModule;
    }
    
    inline const std::shared_ptr<ShaderModule> GetGeomModule() const
    {
        return m_GeomShaderModule;
    }
    
    inline const std::shared_ptr<ShaderModule> GetCompModule() const
    {
        return m_CompShaderModule;
    }
    
    inline const std::shared_ptr<ShaderModule> GetTescModule() const
    {
        return m_TescShaderModule;
    }
    
    inline const std::shared_ptr<ShaderModule> GetTeseModule() const
    {
        return m_TeseShaderModule;
    }
    
    inline const uint32 GetHash() const
    {
        return m_Hash;
    }
    
    inline const std::vector<VkPipelineShaderStageCreateInfo>& GetStageInfo() const
    {
        return m_ShaderCreateInfos;
    }

	inline const VertexInputBindingInfo& GetVertexInputBindingInfo() const
	{
		return m_VertexInputBindingInfo;
	}

	inline const VulkanDescriptorSetsLayout& GetDescriptorSetsLayout() const
	{
		return m_SetsLayoutInfo;
	}

	inline const std::vector<ShaderParamInfo>& GetParams()
	{
		return m_Params;
	}

	void Compile();

protected:

	typedef std::vector<ShaderParamInfo> ShaderParamArray;

	void GenerateHash();

	void ProcessShaderModule(std::shared_ptr<ShaderModule> shaderModule);

protected:

	typedef std::vector<VkPipelineShaderStageCreateInfo> ShaderStageCreateInfoArray;
	
	uint32							m_Hash;
	VulkanDescriptorSetsLayout		m_SetsLayoutInfo;
	VertexInputBindingInfo			m_VertexInputBindingInfo;
	ShaderParamArray				m_Params;

	std::shared_ptr<ShaderModule>	m_VertShaderModule;
	std::shared_ptr<ShaderModule>	m_FragShaderModule;
	std::shared_ptr<ShaderModule>	m_GeomShaderModule;
	std::shared_ptr<ShaderModule>	m_CompShaderModule;
	std::shared_ptr<ShaderModule>	m_TescShaderModule;
	std::shared_ptr<ShaderModule>	m_TeseShaderModule;
    
    ShaderStageCreateInfoArray		m_ShaderCreateInfos;

	static std::unordered_map<std::string, std::shared_ptr<ShaderModule>> g_ShaderModules;
};
