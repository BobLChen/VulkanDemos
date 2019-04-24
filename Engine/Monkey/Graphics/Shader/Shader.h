#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanMemory.h"
#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Texture/TextureBase.h"
#include "Utils/Crc.h"

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

class ShaderModule
{
public:
	ShaderModule(VkShaderModule shaderModule, uint32* dataPtr, uint32 dataSize)
		: m_ShaderModule(shaderModule)
        , m_Data(dataPtr)
        , m_DataSize(dataSize)
        , m_Hash(0)
	{
        m_Hash = Crc::MemCrc32(dataPtr, dataSize);
	}
    
	FORCEINLINE const VkShaderModule& GetHandle() const
	{
		return m_ShaderModule;
	}
    
    FORCEINLINE const uint32* GetData() const
    {
        return m_Data;
    }
    
    FORCEINLINE const uint32 GetDataSize() const
    {
        return m_DataSize;
    }
    
    FORCEINLINE const uint32 GetHash() const
    {
        return m_Hash;
    }

	virtual ~ShaderModule();
    
protected:
	VkShaderModule m_ShaderModule;
    uint32*        m_Data;
    uint32         m_DataSize;
    uint32         m_Hash;
};

class VertexInputBindingInfo
{
public:
	VertexInputBindingInfo()
		: m_Valid(false)
		, m_Hash(0)
	{

	}
    
	FORCEINLINE int32 GetLocation(VertexAttribute attribute) const
	{
		for (int32 i = 0; i < m_Attributes.size(); ++i)
		{
			if (m_Attributes[i] == attribute)
			{
				return m_Locations[i];
			}
		}

        MLOGE("Can't found location, Attribute : %d", attribute);
		return -1;
	}

	FORCEINLINE uint32 GetHash() const
	{
		return m_Hash;
	}
	
	FORCEINLINE void AddBinding(VertexAttribute attribute, int32 location)
	{
		m_Valid = false;
		m_Attributes.push_back(attribute);
		m_Locations.push_back(location);
	}
    
    FORCEINLINE int32 GetInputCount() const
    {
        return int32(m_Attributes.size());
    }
    
	FORCEINLINE void Clear()
	{
		m_Valid = false;
		m_Hash  = 0;
		m_Attributes.clear();
		m_Locations.clear();
	}

	FORCEINLINE void Update()
	{
		if (!m_Valid)
		{
			m_Hash  = Crc::MemCrc32(m_Attributes.data(), int32(m_Attributes.size() * sizeof(int32)), 0);
			m_Hash  = Crc::MemCrc32(m_Locations.data(), int32(m_Locations.size() * sizeof(int32)), m_Hash);
			m_Valid = true;
		}
	}

    FORCEINLINE const std::vector<VertexAttribute>& GetAttributes() const
    {
        return m_Attributes;
    }
    
protected:
	bool                         m_Valid;
	uint32                       m_Hash;
	std::vector<VertexAttribute> m_Attributes;
	std::vector<int32>           m_Locations;
};

class Shader
{

public:

	Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom = nullptr, std::shared_ptr<ShaderModule> comp = nullptr, std::shared_ptr<ShaderModule> tesc = nullptr, std::shared_ptr<ShaderModule> tese = nullptr);

	virtual ~Shader();

	static std::shared_ptr<Shader> Create(const char* vert, const char* frag, const char* geom = nullptr, const char* comp = nullptr, const char* tesc = nullptr, const char* tese = nullptr);

	static std::shared_ptr<ShaderModule> LoadSPIPVShader(const std::string& filename);
    
    FORCEINLINE void Upload()
    {
        if (m_InvalidLayout)
        {
            UpdatePipelineLayout();
        }
    }
    
	FORCEINLINE const VkDescriptorSetLayout& GetDescriptorSetLayout() const
	{
		return m_DescriptorSetLayout;
	}
    
	FORCEINLINE const VkPipelineLayout& GetPipelineLayout() const
	{
		return m_PipelineLayout;
	}

	FORCEINLINE const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages() const
	{
		return m_ShaderStages;
	}
    
	FORCEINLINE const VertexInputBindingInfo& GetVertexInputBindingInfo() const
	{
		return m_VertexInputBindingInfo;
	}

    FORCEINLINE const std::shared_ptr<ShaderModule> GetVertModule() const
    {
        return m_VertShaderModule;
    }
    
    FORCEINLINE const std::shared_ptr<ShaderModule> GetFragModule() const
    {
        return m_FragShaderModule;
    }
    
    FORCEINLINE const std::shared_ptr<ShaderModule> GetGeomModule() const
    {
        return m_GeomShaderModule;
    }
    
    FORCEINLINE const std::shared_ptr<ShaderModule> GetCompModule() const
    {
        return m_CompShaderModule;
    }
    
    FORCEINLINE const std::shared_ptr<ShaderModule> GetTescModule() const
    {
        return m_TescShaderModule;
    }
    
    FORCEINLINE const std::shared_ptr<ShaderModule> GetTeseModule() const
    {
        return m_TeseShaderModule;
    }
    
    FORCEINLINE const uint32 GetHash() const
    {
        return m_Hash;
    }

	FORCEINLINE const std::vector<VkDescriptorPoolSize>& GetPoolSizes() const
	{
		return m_PoolSizes;
	}

protected:
    
    void UpdateVertPipelineLayout();
    
    void UpdateFragPipelineLayout();

	void UpdateCompPipelineLayout();

	void UpdateGeomPipelineLayout();

	void UpdateTescPipelineLayout();

	void UpdateTesePipelineLayout();
    
	void UpdatePipelineLayout();
    
    void DestroyPipelineLayout();
    
private:
    
	uint32												m_Hash;

	bool												m_InvalidLayout;
	VkDescriptorSetLayout								m_DescriptorSetLayout;
	VkPipelineLayout									m_PipelineLayout;
	VertexInputBindingInfo								m_VertexInputBindingInfo;
    
	std::vector<VkPipelineShaderStageCreateInfo>		m_ShaderStages;
    std::vector<VkDescriptorSetLayoutBinding>			m_SetLayoutBindings;
    
	int32												m_DescriptorTypes[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
	std::vector<VkDescriptorPoolSize>					m_PoolSizes;

protected:

	static std::unordered_map<std::string, std::shared_ptr<ShaderModule>> g_ShaderModules;

	std::shared_ptr<ShaderModule> m_VertShaderModule;
	std::shared_ptr<ShaderModule> m_FragShaderModule;
	std::shared_ptr<ShaderModule> m_GeomShaderModule;
	std::shared_ptr<ShaderModule> m_CompShaderModule;
	std::shared_ptr<ShaderModule> m_TescShaderModule;
	std::shared_ptr<ShaderModule> m_TeseShaderModule;
};
