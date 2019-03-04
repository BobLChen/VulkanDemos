#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanMemory.h"
#include "Graphics/Data/VertexBuffer.h"
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
	{
        
	}

	const VkShaderModule& GetHandle() const
	{
		return m_ShaderModule;
	}
    
    const uint32* GetData() const
    {
        return m_Data;
    }
    
    const uint32 GetDataSize() const
    {
        return m_DataSize;
    }

	virtual ~ShaderModule();

protected:
	VkShaderModule m_ShaderModule;
    uint32*        m_Data;
    uint32         m_DataSize;
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
			m_Hash  = Crc::MemCrc32(m_Attributes.data(), m_Attributes.size() * sizeof(int32), 0);
			m_Hash  = Crc::MemCrc32(m_Locations.data(), m_Locations.size() * sizeof(int32), m_Hash);
			m_Valid = true;
		}
	}

protected:
	bool               m_Valid;
	uint32             m_Hash;
	std::vector<int32> m_Attributes;
	std::vector<int32> m_Locations;
};

class Shader
{
private:
    struct UniformBuffer
    {
        VkBuffer		buffer;
        VkDeviceMemory	memory;
        uint32			offset;
        uint32			size;
        uint32			allocationSize;
    };
    
public:

	Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom = nullptr, std::shared_ptr<ShaderModule> comp = nullptr, std::shared_ptr<ShaderModule> tesc = nullptr, std::shared_ptr<ShaderModule> tese = nullptr);

	virtual ~Shader();

	static std::shared_ptr<Shader> Create(const char* vert, const char* frag, const char* geom = nullptr, const char* comp = nullptr, const char* tesc = nullptr, const char* tese = nullptr);

	static std::shared_ptr<ShaderModule> LoadSPIPVShader(const std::string& filename);

    void SetUniformData(const std::string& name, uint8* dataPtr, uint32 dataSize);
    
	FORCEINLINE VkPipelineLayout GetPipelineLayout()
	{
		if (m_InvalidLayout)
		{
			UpdatePipelineLayout();
		}

		return m_PipelineLayout;
	}

	FORCEINLINE const std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages()
	{
		if (m_InvalidLayout)
		{
			UpdatePipelineLayout();
		}

		return m_ShaderStages;
	}
    
	FORCEINLINE const VertexInputBindingInfo& GetVertexInputBindingInfo()
	{
		if (m_InvalidLayout)
		{
			UpdatePipelineLayout();
		}

		return m_VertexInputBindingInfo;
	}

    FORCEINLINE const VkDescriptorSet& GetDescriptorSet()
    {
		if (m_InvalidLayout)
		{
			UpdatePipelineLayout();
		}

        return m_DescriptorSet;
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
    
protected:
    
    void UpdateVertPipelineLayout();
    
    void UpdateFragPipelineLayout();

	void UpdateCompPipelineLayout();

	void UpdateGeomPipelineLayout();

	void UpdateTescPipelineLayout();

	void UpdateTesePipelineLayout();
    
	void UpdatePipelineLayout();
    
    void DestroyPipelineLayout();
	
    void CreateUniformBuffer(UniformBuffer& uniformBuffer, uint32 dataSize, VkBufferUsageFlags usage);
    
private:

	bool					m_InvalidLayout;
	VkPipelineLayout		m_PipelineLayout;
	VkDescriptorPool		m_DescriptorPool;
	VkDescriptorSetLayout	m_DescriptorSetLayout;
	VkDescriptorSet			m_DescriptorSet;
	VertexInputBindingInfo	m_VertexInputBindingInfo;

	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
    std::vector<VkDescriptorSetLayoutBinding>	 m_SetLayoutBindings;
    std::vector<VkDescriptorPoolSize>			 m_PoolSizes;
    std::vector<UniformBuffer>					 m_UniformBuffers;
    std::unordered_map<std::string, int32>		 m_Variables;

protected:

	static std::unordered_map<std::string, std::shared_ptr<ShaderModule>> g_ShaderModules;

	std::shared_ptr<ShaderModule> m_VertShaderModule;
	std::shared_ptr<ShaderModule> m_FragShaderModule;
	std::shared_ptr<ShaderModule> m_GeomShaderModule;
	std::shared_ptr<ShaderModule> m_CompShaderModule;
	std::shared_ptr<ShaderModule> m_TescShaderModule;
	std::shared_ptr<ShaderModule> m_TeseShaderModule;
};
