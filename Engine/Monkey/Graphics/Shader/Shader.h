#pragma once

#include "Common/Common.h"
#include "Common/Log.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanRHI.h"
#include "Vulkan/VulkanMemory.h"
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
    uint32* m_Data;
    uint32 m_DataSize;
};

class Shader
{
private:
    struct UniformBuffer
    {
        VkBuffer buffer;
        VkDeviceMemory memory;
        uint32 offset;
        uint32 size;
        uint32 allocationSize;
    };
    
public:

	Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom = nullptr, std::shared_ptr<ShaderModule> comp = nullptr, std::shared_ptr<ShaderModule> tesc = nullptr, std::shared_ptr<ShaderModule> tese = nullptr);

	virtual ~Shader();

	static std::shared_ptr<Shader> Create(const char* vert, const char* frag, const char* geometry = nullptr, const char* compute = nullptr, const char* tessControl = nullptr, const char* tessEvaluate = nullptr);

	static std::shared_ptr<ShaderModule> LoadSPIPVShader(const std::string& filename);

    void SetUniformData(const std::string& name, uint8* dataPtr, uint32 dataSize);
    
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
    
    FORCEINLINE const VkDescriptorSet& GetDescriptorSet() const
    {
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
    
	void UpdatePipelineLayout();
    
    void DestroyPipelineLayout();
	
    void CreateUniformBuffer(UniformBuffer& uniformBuffer, uint32 dataSize, VkBufferUsageFlags usage);
    
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
    
private:
    std::vector<VkDescriptorSetLayoutBinding> m_SetLayoutBindings;
    std::vector<VkDescriptorPoolSize> m_PoolSizes;
    std::vector<UniformBuffer> m_UniformBuffers;
    std::unordered_map<std::string, int32> m_Variables;
    VkDescriptorPool m_DescriptorPool;
    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkDescriptorSet m_DescriptorSet;
    bool m_Uploaded;
};
