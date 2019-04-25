# Pipelines

之前提到绘制一个模型需要配置整条流水线，设置整条流水线所有需要的参数以及状态。我们可以预先配置好一些Pipelines，然后根据需要选择不同的Pipelines进行绘制。

在这里我先暂时将Pipelines封装为Material，一个Material对应一条Pipeline。上面也提到，我们可以预先配置好一些Pipelines，然后根据需要选择Pipeline。为了优化提高利用率，在Material里面就不单只是根据Material固定生成Pipeline，我这里的处理方式是根据Material的参数以及Shader的原始数据生成对应的Hash，然后通过Hash去Pipeline池里面获取相应的Pipeline。

## Shader

之前的Demo中，加载Shader有大量的代码，为了后期的易用性，在这个Demo里面将Shader简单封装一下。

### ShaderModule

ShaderModule跟Vulkan的ShaderModule对应，我们对VkShaderModule进行简单的包装。我们根据Shader的Binary计算出Hash值，然后保存起数据以及长度。后期可能会为了内存的考量选择将Binary数据卸载掉，在重建的时候再从磁盘重新加载。

```c++
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
    
    const uint32 GetHash() const
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
```

### Shader Input Binding

在Shader里面有许多需要输入的参数，例如顶点属性、UniformBuffer、Texture等等。我们总不能每写一个Shader，就来改一下代码或者新增一个代码片段，特别是对于C++项目来说这完全是灾难。为了解决这个问题，我目前暂时使用了`spirv_cross`这个库，这个是Knronos组织提供的一个Shader反射工具https://github.com/KhronosGroup/SPIRV-Cross。

我们可以利用这个反射库，将Shader里面的参数信息反射出来，然后生成Pipeline相应的绑定信息。目前的解决方案是这样，但是后期我可能更倾向于用Yacc与Lex解析Shader源文件，预先解析好然后生成配置文件。类似于Unity的ShaderLab方式，这里https://github.com/BobLChen/ShaderLab是我之前做的一个库用来解析Unity的ShaderLab，相信后面我们也可以拥有一个自己的ShaderLab，方便引擎的封装与使用。

```c++
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
```

### Shader

整个Pipeline里面需要多种Shader合力支持，因此还需要一个总的Shader封装，将Vertex、Fragment、Compute、tessellation等组织起来。

```c++
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
    
    FORCEINLINE const uint32 GetHash() const
    {
        return m_Hash;
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
    
    uint32                  m_Hash;

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

ShaderModule::~ShaderModule()
{
	if (m_ShaderModule != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle(), m_ShaderModule, VULKAN_CPU_ALLOCATOR);
		m_ShaderModule = VK_NULL_HANDLE;
	}
    
    if (m_Data)
    {
        delete[] m_Data;
        m_Data = nullptr;
    }
}

std::shared_ptr<ShaderModule> Shader::LoadSPIPVShader(const std::string& filename)
{
	auto it = g_ShaderModules.find(filename);
	if (it != g_ShaderModules.end())
	{
		return it->second;
	}

	uint32 dataSize = 0;
	uint8* dataPtr  = nullptr;
	if (!FileManager::ReadFile(filename, dataPtr, dataSize))
	{
		return nullptr;
	}

	VkShaderModuleCreateInfo moduleCreateInfo;
	ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
	moduleCreateInfo.codeSize = dataSize;
	moduleCreateInfo.pCode    = (uint32_t*)dataPtr;

	VkShaderModule shaderModule;
	VERIFYVULKANRESULT(vkCreateShaderModule(Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle(), &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
    
	return std::make_shared<ShaderModule>(shaderModule, (uint32_t*)dataPtr, dataSize);
}

Shader::Shader(std::shared_ptr<ShaderModule> vert, std::shared_ptr<ShaderModule> frag, std::shared_ptr<ShaderModule> geom, std::shared_ptr<ShaderModule> comp, std::shared_ptr<ShaderModule> tesc, std::shared_ptr<ShaderModule> tese)
	: m_InvalidLayout(true)
	, m_PipelineLayout(VK_NULL_HANDLE)
	, m_DescriptorPool(VK_NULL_HANDLE)
	, m_DescriptorSetLayout(VK_NULL_HANDLE)
	, m_DescriptorSet(VK_NULL_HANDLE)
    , m_Hash(0)
	, m_VertShaderModule(vert)
	, m_FragShaderModule(frag)
	, m_GeomShaderModule(geom)
	, m_CompShaderModule(comp)
	, m_TescShaderModule(tesc)
	, m_TeseShaderModule(tese)
{
    uint32 hash0 = Crc::MakeHashCode(
        vert != nullptr ? vert->GetHash() : 0,
        frag != nullptr ? frag->GetHash() : 0,
        geom != nullptr ? geom->GetHash() : 0
    );
    uint32 hash1 = Crc::MakeHashCode(
        comp != nullptr ? comp->GetHash() : 0,
        tesc != nullptr ? tesc->GetHash() : 0,
        tese != nullptr ? tese->GetHash() : 0
    );
    m_Hash = Crc::MakeHashCode(hash0, hash1);
}

Shader::~Shader()
{
	m_VertShaderModule = nullptr;
	m_FragShaderModule = nullptr;
	m_GeomShaderModule = nullptr;
	m_CompShaderModule = nullptr;
	m_TescShaderModule = nullptr;
	m_TeseShaderModule = nullptr;

    DestroyPipelineLayout();
}

std::shared_ptr<Shader> Shader::Create(const char* vert, const char* frag, const char* geom, const char* compute, const char* tesc, const char* tese)
{
	std::shared_ptr<ShaderModule> vertModule = vert ? LoadSPIPVShader(vert) : nullptr;
	std::shared_ptr<ShaderModule> fragModule = frag ? LoadSPIPVShader(frag) : nullptr;
	std::shared_ptr<ShaderModule> geomModule = geom ? LoadSPIPVShader(geom) : nullptr;
	std::shared_ptr<ShaderModule> tescModule = tesc ? LoadSPIPVShader(tesc) : nullptr;
	std::shared_ptr<ShaderModule> teseModule = tese ? LoadSPIPVShader(tese) : nullptr;
	return std::make_shared<Shader>(vertModule, fragModule, geomModule, tescModule, teseModule);
}

void Shader::DestroyPipelineLayout()
{
	if (m_InvalidLayout)
	{
		return;
	}
	m_InvalidLayout = true;

    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
    vkDestroyDescriptorPool(device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
    vkDestroyPipelineLayout(device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
    
    for (int32 i = 0; i < m_UniformBuffers.size(); ++i)
    {
        vkFreeMemory(device, m_UniformBuffers[i].memory, VULKAN_CPU_ALLOCATOR);
        vkDestroyBuffer(device, m_UniformBuffers[i].buffer, VULKAN_CPU_ALLOCATOR);
    }
    
	m_PoolSizes.clear();
	m_Variables.clear();
    m_UniformBuffers.clear();
    m_SetLayoutBindings.clear();
	m_VertexInputBindingInfo.Clear();
}

void Shader::CreateUniformBuffer(UniformBuffer& uniformBuffer, uint32 dataSize, VkBufferUsageFlags usage)
{
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
	// 创建Buffer
    VkBufferCreateInfo bufferCreateInfo;
    ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    bufferCreateInfo.size  = dataSize;
    bufferCreateInfo.usage = usage;
    VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferCreateInfo, VULKAN_CPU_ALLOCATOR, &uniformBuffer.buffer));
    
	// 获取内存分配信息
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, uniformBuffer.buffer, &memReqs);
    uint32 memoryTypeIndex = 0;
    Engine::Get()->GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
    
    VkMemoryAllocateInfo allocInfo = {};
    ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    allocInfo.allocationSize  = memReqs.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

	// 分配内存并绑定
    VERIFYVULKANRESULT(vkAllocateMemory(device, &allocInfo, VULKAN_CPU_ALLOCATOR, &uniformBuffer.memory));
    VERIFYVULKANRESULT(vkBindBufferMemory(device, uniformBuffer.buffer, uniformBuffer.memory, 0));
    
	// 记录分配信息
	uniformBuffer.size = dataSize;
	uniformBuffer.offset = 0;
    uniformBuffer.allocationSize = uint32(memReqs.size);
}

void Shader::UpdateFragPipelineLayout()
{
	if (m_FragShaderModule == nullptr)
	{
		return;
	}

	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	stageInfo.module = m_FragShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateCompPipelineLayout()
{
	if (m_CompShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
	stageInfo.module = m_CompShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateGeomPipelineLayout()
{
	if (m_GeomShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
	stageInfo.module = m_GeomShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateTescPipelineLayout()
{
	if (m_TescShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	stageInfo.module = m_TescShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateTesePipelineLayout()
{
	if (m_TeseShaderModule == nullptr)
	{
		return;
	}
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	stageInfo.module = m_TeseShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);
}

void Shader::UpdateVertPipelineLayout()
{
    if (m_VertShaderModule == nullptr)
    {
        return;
    }

	// 保存StageInfo
	VkPipelineShaderStageCreateInfo stageInfo;
	ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
	stageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
	stageInfo.module = m_VertShaderModule->GetHandle();
	stageInfo.pName  = "main";
	m_ShaderStages.push_back(stageInfo);

	// 反编译Shader获取相关信息
    spirv_cross::Compiler compiler(m_VertShaderModule->GetData(), m_VertShaderModule->GetDataSize() / sizeof(uint32));
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();
    
	// 获取Uniform Buffer信息
    for (int32 i = 0; i < resources.uniform_buffers.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.uniform_buffers[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string &varName      = compiler.get_name(res.id);
        
        VkDescriptorSetLayoutBinding uboBinding = {};
        uboBinding.binding = compiler.get_decoration(res.id, spv::DecorationBinding);
        uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboBinding.descriptorCount = 1;
        uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboBinding.pImmutableSamplers = nullptr;
        m_SetLayoutBindings.push_back(uboBinding);
        
        VkDescriptorPoolSize poolSize;
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 1;
        m_PoolSizes.push_back(poolSize);
        
        UniformBuffer uniformBuffer;
        CreateUniformBuffer(uniformBuffer, uint32(compiler.get_declared_struct_size(base_type)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
        
		m_Variables.insert(std::make_pair(varName, m_UniformBuffers.size()));
        m_UniformBuffers.push_back(uniformBuffer);
    }
    
	// 获取Input Location信息
    for (int32 i = 0; i < resources.stage_inputs.size(); ++i)
    {
        spirv_cross::Resource& res = resources.stage_inputs[i];
        const std::string &varName = compiler.get_name(res.id);
        VertexAttribute attribute  = StringToVertexAttribute(varName.c_str());
		m_VertexInputBindingInfo.AddBinding(attribute, compiler.get_decoration(res.id, spv::DecorationLocation));
    }

	m_VertexInputBindingInfo.Update();
}

void Shader::UpdatePipelineLayout()
{
	if (!m_InvalidLayout) 
	{
		return;
	}

    DestroyPipelineLayout();

    UpdateVertPipelineLayout();
	UpdateGeomPipelineLayout();
	UpdateTescPipelineLayout();
	UpdateTesePipelineLayout();
	UpdateCompPipelineLayout();
    UpdateFragPipelineLayout();
    
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    
    // 创建SetLayout
    VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
    ZeroVulkanStruct(setLayoutCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
    setLayoutCreateInfo.bindingCount = uint32_t(m_SetLayoutBindings.size());
    setLayoutCreateInfo.pBindings    = m_SetLayoutBindings.data();
    VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &setLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));
    
    // 创建Pool
    VkDescriptorPoolCreateInfo poolCreateInfo;
    ZeroVulkanStruct(poolCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
	poolCreateInfo.maxSets       = 1;
    poolCreateInfo.poolSizeCount = uint32_t(m_PoolSizes.size());
    poolCreateInfo.pPoolSizes    = m_PoolSizes.data();
    VERIFYVULKANRESULT(vkCreateDescriptorPool(device, &poolCreateInfo, VULKAN_CPU_ALLOCATOR,  &m_DescriptorPool));
    
    // 分配真正的Set
    VkDescriptorSetAllocateInfo setAllococateInfo;
    ZeroVulkanStruct(setAllococateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
    setAllococateInfo.descriptorPool     = m_DescriptorPool;
    setAllococateInfo.descriptorSetCount = 1;
    setAllococateInfo.pSetLayouts        = &m_DescriptorSetLayout;
    VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &setAllococateInfo, &m_DescriptorSet));
    
	// 创建Uniform Buffer Info
    std::vector<VkDescriptorBufferInfo> bufferInfos(m_UniformBuffers.size());
    for (int32 i = 0; i < m_UniformBuffers.size(); ++i)
    {
        bufferInfos[i].buffer = m_UniformBuffers[i].buffer;
        bufferInfos[i].offset = 0;
        bufferInfos[i].range  = m_UniformBuffers[i].size;
    }
    
	// 更新Descriptor Set
    std::vector<VkWriteDescriptorSet> descriptorWrites(m_UniformBuffers.size());
    for (int32 i = 0; i < descriptorWrites.size(); ++i)
    {
        ZeroVulkanStruct(descriptorWrites[i], VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        descriptorWrites[i].dstSet          = m_DescriptorSet;
        descriptorWrites[i].dstBinding      = m_SetLayoutBindings[i].binding;
        descriptorWrites[i].dstArrayElement = 0;
        descriptorWrites[i].descriptorType  = m_SetLayoutBindings[i].descriptorType;
        descriptorWrites[i].descriptorCount = 1;
        descriptorWrites[i].pBufferInfo     = &bufferInfos[i];
    }
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, VULKAN_CPU_ALLOCATOR);
    
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    ZeroVulkanStruct(pipelineLayoutCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts    = &m_DescriptorSetLayout;
    VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));

	m_InvalidLayout = false;
}

void Shader::SetUniformData(const std::string& name, uint8* dataPtr, uint32 dataSize)
{
    auto it = m_Variables.find(name);
    if (it == m_Variables.end())
    {
        return;
    }
    
    int32 index = it->second;
    if (index < 0 || index >= m_UniformBuffers.size())
    {
        return;
    }
    
    VkDevice device = Engine::Get()->GetVulkanRHI()->GetDevice()->GetInstanceHandle();
    UniformBuffer& uniformBuffer = m_UniformBuffers[index];

	// TODO:只Map一次，销毁时Unmap
    uint8_t *pData = nullptr;
    VERIFYVULKANRESULT(vkMapMemory(device, uniformBuffer.memory, 0, dataSize, 0, (void**)&pData));
    std::memcpy(pData, dataPtr, dataSize);
    vkUnmapMemory(device, uniformBuffer.memory);
}
```

在这里，我只暂时处理了一下UniformBuffer的参数，因为到现在我们也只是需要UniformBuffer的数据，后面其它的数据会在后期的Demo里面逐步完善，也有可能后期的Demo会让我们修改这个结构。但是目前来说我们这样封装就好。

## Material

Material可以使用不同的Shader，不同的Material也可以使用同一个Shader，唯一的区别只是参数不一样而已。这个Demo之后我们就需要考虑如何组织这些参数，因为这些参数不同的绘制对象不同，每一个缓冲区的数据也不同，我们需要有效的组织起来，传递给GPU绘制。

目前Material里面可以获取VkPipeline，VkPipeline也就是之前提到的，通过参数生成Key，通过Key获取正确的Pipeline，提高利用率。

```c++
VkPipeline Material::GetPipeline(const VertexInputDeclareInfo& inputDeclareInfo, const VertexInputBindingInfo& inputBindingInfo)
{
	VkPipeline pipelineResult = VK_NULL_HANDLE;
	std::shared_ptr<VulkanRHI> vulkanRHI = Engine::Get()->GetVulkanRHI();
	
    if (inputDeclareInfo.GetAttributes().size() != inputBindingInfo.GetAttributes().size())
    {
        MLOGE(
              "VertexBuffer(%d) not match VertexShader(%d).",
              (int32)inputDeclareInfo.GetAttributes().size(),
              (int32)inputBindingInfo.GetAttributes().size()
        );
        return pipelineResult;
    }
    // TODO:快速校验VertexData格式是否匹配VertexShader
    
	uint32 hash0 = inputDeclareInfo.GetHash();
	uint32 hash1 = inputBindingInfo.GetHash();

	// Pipeline发生修改，重新计算Pipeline的Key
	if (m_InvalidPipeline)
	{
		m_InvalidPipeline = false;
		m_MultisampleState.rasterizationSamples = vulkanRHI->GetSampleCount();
		m_Hash = Crc::StrCrc32((const char*)&m_InputAssemblyState, sizeof(VkPipelineInputAssemblyStateCreateInfo), m_Shader->GetHash());
		m_Hash = Crc::StrCrc32((const char*)&m_RasterizationState, sizeof(VkPipelineRasterizationStateCreateInfo), m_Hash);
		m_Hash = Crc::StrCrc32((const char*)&m_ColorBlendAttachmentState, sizeof(VkPipelineColorBlendAttachmentState), m_Hash);
		m_Hash = Crc::StrCrc32((const char*)&m_ViewportState, sizeof(VkPipelineViewportStateCreateInfo), m_Hash);
		m_Hash = Crc::StrCrc32((const char*)&m_DepthStencilState, sizeof(VkPipelineDepthStencilStateCreateInfo), m_Hash);
		m_Hash = Crc::StrCrc32((const char*)&m_MultisampleState, sizeof(VkPipelineMultisampleStateCreateInfo), m_Hash);
	}
    
	uint32 hash = Crc::MakeHashCode(hash0, hash1, m_Hash);
	auto it = G_PipelineCache.find(hash);

	// 缓存中不存在Pipeline，创建一个新的
	if (it == G_PipelineCache.end())
	{
		VkPipelineColorBlendStateCreateInfo colorBlendState;
		ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments    = &m_ColorBlendAttachmentState;

		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
		dynamicState.dynamicStateCount = uint32_t(dynamicStateEnables.size());
		dynamicState.pDynamicStates    = dynamicStateEnables.data();
		
		const std::vector<VertexInputDeclareInfo::BindingDescription>& inVertexBindings = inputDeclareInfo.GetBindings();
		std::vector<VkVertexInputBindingDescription> vertexInputBindings(inVertexBindings.size());
		for (int32 i = 0; i < vertexInputBindings.size(); ++i)
		{
			vertexInputBindings[i].binding   = inVertexBindings[i].binding;
			vertexInputBindings[i].stride    = inVertexBindings[i].stride;
			vertexInputBindings[i].inputRate = inVertexBindings[i].inputRate;
		}

		const std::vector<VertexInputDeclareInfo::AttributeDescription>& inVertexDescInfos = inputDeclareInfo.GetAttributes();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(inVertexDescInfos.size());
		for (int32 i = 0; i < vertexInputAttributs.size(); ++i)
		{
			vertexInputAttributs[i].binding  = inVertexDescInfos[i].binding;
			vertexInputAttributs[i].location = inputBindingInfo.GetLocation(inVertexDescInfos[i].attribute);
			vertexInputAttributs[i].format   = inVertexDescInfos[i].format;
			vertexInputAttributs[i].offset   = inVertexDescInfos[i].offset;
		}
        
		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount   = uint32_t(vertexInputBindings.size());
		vertexInputState.pVertexBindingDescriptions      = vertexInputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = uint32_t(vertexInputAttributs.size());
		vertexInputState.pVertexAttributeDescriptions    = vertexInputAttributs.data();

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout              = m_Shader->GetPipelineLayout();
		pipelineCreateInfo.renderPass          = vulkanRHI->GetRenderPass();
		pipelineCreateInfo.stageCount          = uint32_t(m_Shader->GetShaderStages().size());
		pipelineCreateInfo.pStages             = m_Shader->GetShaderStages().data();
		pipelineCreateInfo.pVertexInputState   = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &m_InputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &m_RasterizationState;
		pipelineCreateInfo.pColorBlendState    = &colorBlendState;
		pipelineCreateInfo.pMultisampleState   = &m_MultisampleState;
		pipelineCreateInfo.pViewportState      = &m_ViewportState;
		pipelineCreateInfo.pDepthStencilState  = &m_DepthStencilState;
		pipelineCreateInfo.pDynamicState       = &dynamicState;

		VkPipeline pipeline = VK_NULL_HANDLE;
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(vulkanRHI->GetDevice()->GetInstanceHandle(), vulkanRHI->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &pipeline));
		pipelineResult = pipeline;
		G_PipelineCache.insert(std::make_pair(hash, pipeline));
        
        MLOG("Add GraphicsPipelines[%ud] to cache.", hash);
	}
	else 
	{
		pipelineResult = it->second;
	}
    
	return pipelineResult;
}
```

## Pipeline

现在我们回过头来编写我们的Demo，经过了一系列的封装，现在Demo代码量就会急剧减少。

```c++
#include "Common/Common.h"
#include "Common/Log.h"
#include "Configuration/Platform.h"
#include "Application/AppModeBase.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanQueue.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanMemory.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Loader/MeshLoader.h"
#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Data/IndexBuffer.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Renderer/Mesh.h"
#include "Graphics/Renderer/Renderable.h"
#include "File/FileManager.h"
#include <vector>

typedef std::shared_ptr<Mesh>		MeshPtr;
typedef std::shared_ptr<Shader>		ShaderPtr;
typedef std::shared_ptr<Material>	MaterialPtr;

class Pipelines : public AppModeBase
{
public:
    Pipelines(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_Ready(false)
		, m_CurrentBackBuffer(0)
        , m_RenderComplete(VK_NULL_HANDLE)
    {

    }
    
    virtual ~Pipelines()
    {
        
    }
    
    virtual void PreInit() override
    {
        
    }
    
    virtual void Init() override
    {
		// 准备MVP数据
		m_MVPData.model.SetIdentity();
		m_MVPData.view.SetIdentity();
		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetWidth(), (float)GetHeight(), 0.01f, 3000.0f);

		// 加载Mesh
        LoadAssets();

		// 创建同步对象
		CreateSynchronousObject();

		// 录制Command命令
        SetupCommandBuffers();
        
        m_Ready = true;
    }
    
    virtual void Exist() override
    {
        DestroySynchronousObject();
		m_Meshes.clear();
        Material::DestroyCache();
    }
    
    virtual void Loop() override
    {
        if (m_Ready)
        {
            UpdateUniformBuffers();
			Draw();
        }
    }
    
private:
    
    struct UBOData
    {
        Matrix4x4 model;
        Matrix4x4 view;
        Matrix4x4 projection;
    };
    
    void Draw()
    {
		std::shared_ptr<VulkanRHI> vulkanRHI         = GetVulkanRHI();
		VkPipelineStageFlags waitStageMask           = vulkanRHI->GetStageMask();
		std::shared_ptr<VulkanQueue> gfxQueue        = vulkanRHI->GetDevice()->GetGraphicsQueue();
		std::shared_ptr<VulkanQueue> presentQueue    = vulkanRHI->GetDevice()->GetPresentQueue();
		std::shared_ptr<VulkanSwapChain> swapChain   = vulkanRHI->GetSwapChain();
		std::vector<VkCommandBuffer>& drawCmdBuffers = vulkanRHI->GetCommandBuffers();
		VulkanFenceManager& fenceMgr                 = GetVulkanRHI()->GetDevice()->GetFenceManager();
        VkSemaphore waitSemaphore                    = VK_NULL_HANDLE;
        m_CurrentBackBuffer                          = swapChain->AcquireImageIndex(&waitSemaphore);

        fenceMgr.WaitForFence(m_Fences[m_CurrentBackBuffer], MAX_uint64);
        fenceMgr.ResetFence(m_Fences[m_CurrentBackBuffer]);
        
        VkSubmitInfo submitInfo = {};
        submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask	= &waitStageMask;
        submitInfo.pWaitSemaphores		= &waitSemaphore;
        submitInfo.waitSemaphoreCount	= 1;
        submitInfo.pSignalSemaphores	= &m_RenderComplete;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pCommandBuffers		= &drawCmdBuffers[m_CurrentBackBuffer];
        submitInfo.commandBufferCount	= 1;
        
        VERIFYVULKANRESULT(vkQueueSubmit(gfxQueue->GetHandle(), 1, &submitInfo, m_Fences[m_CurrentBackBuffer]->GetHandle()));
        swapChain->Present(gfxQueue, presentQueue, &m_RenderComplete);
    }

	void BindMeshCommand(std::shared_ptr<Mesh> mesh, VkCommandBuffer command)
	{
		std::vector<std::shared_ptr<Renderable>> renderables = mesh->GetRenderables();
		std::vector<std::shared_ptr<Material>> materials = mesh->GetMaterials();

		for (int32 i = 0; i < renderables.size(); ++i)
		{
			std::shared_ptr<Renderable> renderable = renderables[i];
			std::shared_ptr<Material> material = materials[i];
			std::shared_ptr<Shader> shader = material->GetShader();

			if (!renderable->IsValid())
			{
				continue;
			}
			
			const VertexInputDeclareInfo& vertInfo = renderable->GetVertexBuffer()->GetVertexInputStateInfo();
			VkPipeline pipeline = material->GetPipeline(vertInfo, shader->GetVertexInputBindingInfo());

			vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetPipelineLayout(), 0, 1, &(shader->GetDescriptorSet()), 0, nullptr);
			vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			
			renderable->BindBufferToCommand(command);
			renderable->BindDrawToCommand(command);
		}

	}
    
    void SetupCommandBuffers()
    {
		std::shared_ptr<VulkanRHI> vulkanRHI = GetVulkanRHI();
        
        VkCommandBufferBeginInfo cmdBeginInfo;
        ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
        
        VkClearValue clearValues[2];
        clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

		uint32 width  = vulkanRHI->GetSwapChain()->GetWidth();
		uint32 height = vulkanRHI->GetSwapChain()->GetHeight();
        
        VkRenderPassBeginInfo renderPassBeginInfo;
        ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = vulkanRHI->GetRenderPass();
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues    = clearValues;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width  = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        
        std::vector<VkCommandBuffer>& drawCmdBuffers = vulkanRHI->GetCommandBuffers();
        std::vector<VkFramebuffer>& frameBuffers     = vulkanRHI->GetFrameBuffers();
        
        for (int32 i = 0; i < drawCmdBuffers.size(); ++i)
        {
            renderPassBeginInfo.framebuffer = frameBuffers[i];
            
            VERIFYVULKANRESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBeginInfo));
            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            
			for (int32 j = 0; j < m_Meshes.size(); ++j)
			{
				VkViewport viewport = {};
				viewport.width  = 0.5f * width;
				viewport.height = 0.5f * height;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				viewport.x = (j % 2) * viewport.width;
				viewport.y = (j / 2) * viewport.height;

				VkRect2D scissor = {};
				scissor.extent.width  = viewport.width;
				scissor.extent.height = viewport.height;
				scissor.offset.x = viewport.x;
				scissor.offset.y = viewport.y;

				vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
				vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

				BindMeshCommand(m_Meshes[j], drawCmdBuffers[i]);
			}

			vkCmdEndRenderPass(drawCmdBuffers[i]);
            VERIFYVULKANRESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }
    
    void UpdateUniformBuffers()
    {
		m_MVPData.model.AppendRotation(0.1f, Vector3::UpVector);

        m_MVPData.view.SetIdentity();
        m_MVPData.view.SetOrigin(Vector4(0, -2.5f, 30.0f));
        m_MVPData.view.AppendRotation(15.0f, Vector3::RightVector);
        m_MVPData.view.SetInverse();
        
		for (int32 i = 0; i < m_Meshes.size(); ++i)
		{
			const std::vector<MaterialPtr>& materials = m_Meshes[i]->GetMaterials();
			for (int32 j = 0; j < materials.size(); ++j)
			{
				materials[j]->GetShader()->SetUniformData("uboMVP", (uint8*)&m_MVPData, sizeof(UBOData));
			}
		}
    }
    
    void LoadAssets()
    {
		// 加载Shader以及Material
		ShaderPtr   shader0 = Shader::Create("assets/shaders/4_Pipelines/phong.vert.spv", "assets/shaders/4_Pipelines/phong.frag.spv");
		MaterialPtr material0 = std::make_shared<Material>(shader0);
		ShaderPtr	shader1 = Shader::Create("assets/shaders/4_Pipelines/pipelines.vert.spv", "assets/shaders/4_Pipelines/pipelines.frag.spv");
		MaterialPtr material1 = std::make_shared<Material>(shader1);
		ShaderPtr	shader2 = Shader::Create("assets/shaders/4_Pipelines/solid.vert.spv", "assets/shaders/4_Pipelines/solid.frag.spv");
		MaterialPtr	material2 = std::make_shared<Material>(shader2);
		ShaderPtr	shader3 = Shader::Create("assets/shaders/4_Pipelines/solid.vert.spv", "assets/shaders/4_Pipelines/solid.frag.spv");
		MaterialPtr	material3 = std::make_shared<Material>(shader3);
		material3->SetpolygonMode(VkPolygonMode::VK_POLYGON_MODE_LINE);
		
		std::vector<MaterialPtr> materials(4);
		materials[0] = material0;
		materials[1] = material1;
		materials[2] = material2;
		materials[3] = material3;
		// 加载模型
		std::vector<std::shared_ptr<Renderable>> renderables = OBJMeshParser::LoadFromFile("assets/models/suzanne.obj");
		
		for (int32 i = 0; i < materials.size(); ++i)
		{
			MeshPtr mesh = std::make_shared<Mesh>();
			for (int32 j = 0; j < renderables.size(); ++j)
			{
				mesh->AddSubMesh(renderables[j], materials[i]);
			}
			m_Meshes.push_back(mesh);
		}
    }
    
    void CreateSynchronousObject()
    {
		std::shared_ptr<VulkanRHI> vulkanRHI = GetVulkanRHI();
		VkDevice device = vulkanRHI->GetDevice()->GetInstanceHandle();

        m_Fences.resize(GetVulkanRHI()->GetSwapChain()->GetBackBufferCount());
        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        for (int32 index = 0; index < m_Fences.size(); ++index)
        {
            m_Fences[index] = fenceMgr.CreateFence(true);
        }
        
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		vkCreateSemaphore(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
    }
    
    void DestroySynchronousObject()
    {
		std::shared_ptr<VulkanRHI> vulkanRHI = GetVulkanRHI();
		VkDevice device = vulkanRHI->GetDevice()->GetInstanceHandle();

        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        for (int32 index = 0; index < m_Fences.size(); ++index)
        {
            fenceMgr.WaitAndReleaseFence(m_Fences[index], MAX_int64);
        }
        m_Fences.clear();

		vkDestroySemaphore(device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
    }

private:
    UBOData                       m_MVPData;
    bool                          m_Ready;
    uint32                        m_CurrentBackBuffer;
	std::vector<MeshPtr>		  m_Meshes;
    VkSemaphore                   m_RenderComplete;
    std::vector<VulkanFence*>     m_Fences;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
    return new Pipelines(1120, 840, "Pipelines", cmdLine);
}

```

在这里我们使用四个不同的Material去绘制模型，呈现出了四种不同的状态。