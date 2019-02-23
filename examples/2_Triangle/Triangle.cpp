#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/AppModeBase.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanQueue.h"
#include "Math/Vector4.h"
#include <vector>

class TriangleMode : public AppModeBase
{
public:
	TriangleMode(int width, int height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_CmdLine(cmdLine)
	{

	}

	virtual ~TriangleMode()
	{

	}

	virtual void PreInit() override
	{
		std::string assetsPath = m_CmdLine[0];
		int32 length = 0;
		for (int32 i = assetsPath.size() - 1; i >= 0; --i)
		{
			if (assetsPath[i] == '/')
			{
				break;
			}
			length += 1;
		}
		m_AssetsPath = assetsPath.substr(0, assetsPath.size() - length);
	}

	virtual void Init() override
	{
		m_VulkanRHI = GetVulkanRHI();
		m_Device    = m_VulkanRHI->GetDevice()->GetInstanceHandle();

		CreateFences();
		CreateMeshBuffers();
		CreateUniformBuffers();
		CreateDescriptorSetLayout();
		CreatePipelines();
	}

	virtual void Loop() override
	{

	}

	virtual void Exist() override
	{
		DestroyPipelines();
		DestroyDescriptorSetLayout();
		DestroyUniformBuffers();
		DestroyMeshBuffers();
		DestroyFences();
	}

private:

	struct GPUBuffer
	{
		VkDeviceMemory memory;
		VkBuffer buffer;
	};

	typedef GPUBuffer IndexBuffer;
	typedef GPUBuffer VertexBuffer;
	typedef GPUBuffer UBOBuffer;

	struct Vertex
	{
		float position[3];
		float color[3];
	};

	struct UBOData
	{
		float datas[48];
	};

	VkShaderModule LoadSPIPVShader(std::string filepath)
	{
		std::string finalPath = m_AssetsPath + "../../examples/" + filepath;
		FILE* file = fopen(finalPath.c_str(), "rt");
		if (!file)
		{
			MLOGE("File not found :%s", filepath.c_str());
			return VK_NULL_HANDLE;
		}

		fseek(file, 0, SEEK_END);
		int32 dataSize = ftell(file);
		fseek(file, 0, SEEK_SET);

		if (dataSize <= 0)
		{
			fclose(file);
			MLOGE("File has no data :%s", filepath.c_str());
			return VK_NULL_HANDLE;
		}

		uint8* data = new uint8[dataSize];
		fread(data, 1, dataSize, file);
		fclose(file);

		VkShaderModuleCreateInfo moduleCreateInfo;
		ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
		moduleCreateInfo.codeSize = dataSize;
		moduleCreateInfo.pCode = (uint32_t*)data;

		VkShaderModule shaderModule;
		VERIFYVULKANRESULT(vkCreateShaderModule(m_Device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
		delete[] data;
		
		return shaderModule;
	}

	void CreatePipelines()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
		VkPipelineRasterizationStateCreateInfo rasterizationState;
		ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
		rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode = VK_CULL_MODE_NONE;
		rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable = VK_FALSE;
		rasterizationState.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
		blendAttachmentState[0].colorWriteMask = 0xF;
		blendAttachmentState[0].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendState;
		ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = blendAttachmentState;

		VkPipelineViewportStateCreateInfo viewportState;
		ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
		viewportState.viewportCount = 1;
		viewportState.scissorCount  = 1;

		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
		VkPipelineDynamicStateCreateInfo dynamicState;
		ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
		dynamicState.dynamicStateCount = dynamicStateEnables.size();
		dynamicState.pDynamicStates = dynamicStateEnables.data();

		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
		depthStencilState.depthTestEnable = VK_TRUE;
		depthStencilState.depthWriteEnable = VK_TRUE;
		depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable = VK_FALSE;
		depthStencilState.front = depthStencilState.back;

		VkPipelineMultisampleStateCreateInfo multisampleState;
		ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
		multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleState.pSampleMask = nullptr;
		
		// (triangle.vert):
		// layout (location = 0) in vec3 inPos;
		// layout (location = 1) in vec3 inColor;
		// Attribute location 0: Position
		// Attribute location 1: Color
		// vertex input bindding
		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding = 0;
		vertexInputBinding.stride = sizeof(Vertex);
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(2);
		// position
		vertexInputAttributs[0].binding = 0;
		vertexInputAttributs[0].location = 0;
		vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[0].offset = offsetof(Vertex, position);
		// color
		vertexInputAttributs[1].binding = 0;
		vertexInputAttributs[1].location = 1;
		vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[1].offset = offsetof(Vertex, color);
		
		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount = 1;
		vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
		vertexInputState.vertexAttributeDescriptionCount = 2;
		vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
		ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = LoadSPIPVShader("assets/shaders/2_Triangle/triangle.vert.spv");
		shaderStages[0].pName = "main";
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = LoadSPIPVShader("assets/shaders/2_Triangle/triangle.frag.spv");
		shaderStages[1].pName = "main";

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout = m_PipelineLayout;
		pipelineCreateInfo.renderPass = m_VulkanRHI->GetRenderPass();
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(m_Device, m_VulkanRHI->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));
		
		vkDestroyShaderModule(m_Device, shaderStages[0].module, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, shaderStages[1].module, VULKAN_CPU_ALLOCATOR);
	}

	void DestroyPipelines()
	{
		
	}
	
	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding = 0;
		layoutBinding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount = 1;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 1;
		descSetLayoutInfo.pBindings = &layoutBinding;
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(m_Device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));

		VkPipelineLayoutCreateInfo pipeLayoutInfo;
		ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipeLayoutInfo.setLayoutCount = 1;
		pipeLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkCreatePipelineLayout(m_Device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
	}

	void DestroyDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
	}
	
	void UpdateUniformBuffers()
	{
		for (int i = 0; i < 48; ++i)
		{
			m_MVPData.datas[i] = 0;
		}
		
		uint8_t *pData = nullptr;
		VERIFYVULKANRESULT(vkMapMemory(m_Device, m_MVPBuffer.memory, 0, 48 * sizeof(float), 0, (void**)&pData));
		std::memcpy(pData, m_MVPData.datas, 48 * sizeof(float));
		vkUnmapMemory(m_Device, m_MVPBuffer.memory);
	}

	void CreateUniformBuffers()
	{
		VkBufferCreateInfo bufferInfo;
		ZeroVulkanStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		bufferInfo.size = 48 * sizeof(float);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &bufferInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.buffer));

		VkMemoryRequirements memReqInfo;
		vkGetBufferMemoryRequirements(m_Device, m_MVPBuffer.buffer, &memReqInfo);
		uint32 memoryTypeIndex = 0;
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);

		VkMemoryAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		allocInfo.allocationSize  = memReqInfo.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &allocInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.memory));

		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_MVPBuffer.buffer, m_MVPBuffer.memory, 0));
		m_MVPDescriptor.buffer = m_MVPBuffer.buffer;
		m_MVPDescriptor.offset = 0;
		m_MVPDescriptor.range  = 48 * sizeof(float);

		UpdateUniformBuffers();
	}
	
	void DestroyUniformBuffers()
	{
		vkDestroyBuffer(m_Device, m_MVPBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, m_MVPBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void CreateMeshBuffers()
	{
		// 顶点数据
		std::vector<Vertex> vertices = {
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};

		// 索引数据
		std::vector<uint16> indices = { 0, 1, 2 };
		m_IndicesCount = indices.size();

		// 顶点数据以及索引数据在整个生命周期中几乎不会发生改变，因此最佳的方式是将这些数据存储到GPU的内存中。
		// 存储到GPU内存也能加快GPU的访问。为了存储到GPU内存中，需要如下几个步骤。
		// 1、在主机端(Host)创建一个Buffer
		// 2、将数据拷贝至该Buffer
		// 3、在GPU端(Local Device)创建一个Buffer
		// 4、通过Transfer簇将数据从主机端拷贝至GPU端
		// 5、删除主基端(Host)的Buffer
		// 6、使用GPU端(Local Device)的Buffer进行渲染
		VertexBuffer tempVertexBuffer;
		IndexBuffer tempIndexBuffer;

		void* dataPtr = nullptr;
		VkMemoryRequirements memReqInfo;
		VkMemoryAllocateInfo memAllocInfo;
		ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

		// vertex buffer
		VkBufferCreateInfo vertexBufferInfo;
		ZeroVulkanStruct(vertexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		vertexBufferInfo.size  = vertices.size() * sizeof(Vertex);
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.buffer));

		vkGetBufferMemoryRequirements(m_Device, tempVertexBuffer.buffer, &memReqInfo);
		uint32 memoryTypeIndex = 0;
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.memory));
		
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, tempVertexBuffer.buffer, tempVertexBuffer.memory, 0));

		VERIFYVULKANRESULT(vkMapMemory(m_Device, tempVertexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, vertices.data(), vertexBufferInfo.size);
		vkUnmapMemory(m_Device, tempVertexBuffer.memory);

		// local device vertex buffer
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.buffer));

		vkGetBufferMemoryRequirements(m_Device, m_VertexBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_VertexBuffer.buffer, m_VertexBuffer.memory, 0));

		// index buffer
		VkBufferCreateInfo indexBufferInfo;
		ZeroVulkanStruct(indexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		indexBufferInfo.size  = m_IndicesCount * sizeof(uint16);
		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.buffer));

		vkGetBufferMemoryRequirements(m_Device, tempIndexBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.memory));
		
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, tempIndexBuffer.buffer, tempIndexBuffer.memory, 0));

		VERIFYVULKANRESULT(vkMapMemory(m_Device, tempIndexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, indices.data(), indexBufferInfo.size);
		vkUnmapMemory(m_Device, tempIndexBuffer.memory);
		
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.buffer));
		
		vkGetBufferMemoryRequirements(m_Device, m_IndicesBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_IndicesBuffer.buffer, m_IndicesBuffer.memory, 0));

		VkCommandBuffer xferCmdBuffer;
		// gfx queue自带transfer功能，为了优化需要使用转悠的xfer queue。这里为了简单，先将就用。
		VkCommandBufferAllocateInfo xferCmdBufferInfo;
		ZeroVulkanStruct(xferCmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
		xferCmdBufferInfo.commandPool = m_VulkanRHI->GetCommandPool();
		xferCmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		xferCmdBufferInfo.commandBufferCount = 1;
		VERIFYVULKANRESULT(vkAllocateCommandBuffers(m_Device, &xferCmdBufferInfo, &xferCmdBuffer));

		// 开始录制命令
		VkCommandBufferBeginInfo cmdBufferBeginInfo;
		ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(xferCmdBuffer, &cmdBufferBeginInfo));
		
		VkBufferCopy copyRegion = {};
		copyRegion.size = vertices.size() * sizeof(Vertex);
		vkCmdCopyBuffer(xferCmdBuffer, tempVertexBuffer.buffer, m_VertexBuffer.buffer, 1, &copyRegion);
		
		copyRegion.size = indices.size() * sizeof(uint16);
		vkCmdCopyBuffer(xferCmdBuffer, tempIndexBuffer.buffer, m_IndicesBuffer.buffer, 1, &copyRegion);

		// 结束录制
		VERIFYVULKANRESULT(vkEndCommandBuffer(xferCmdBuffer));
		
		// 提交命令，并且等待命令执行完毕。
		VkSubmitInfo submitInfo;
		ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &xferCmdBuffer;

		VkFenceCreateInfo fenceInfo = {};
		ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceInfo.flags = 0;

		VkFence fence;
		VERIFYVULKANRESULT(vkCreateFence(m_Device, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));
		VERIFYVULKANRESULT(vkQueueSubmit(m_VulkanRHI->GetDevice()->GetGraphicsQueue()->GetHandle(), 1, &submitInfo, fence));
		VERIFYVULKANRESULT(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, MAX_int64));

		vkDestroyFence(m_Device, fence, VULKAN_CPU_ALLOCATOR);
		vkFreeCommandBuffers(m_Device, m_VulkanRHI->GetCommandPool(), 1, &xferCmdBuffer);

		vkDestroyBuffer(m_Device, tempVertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, tempVertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(m_Device, tempIndexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, tempIndexBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void DestroyMeshBuffers()
	{
		vkDestroyBuffer(m_Device, m_VertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, m_VertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(m_Device, m_IndicesBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, m_IndicesBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void CreateFences()
	{
		m_Fences.resize(m_VulkanRHI->GetCommandBuffers().size());

		VkFenceCreateInfo fenceCreateInfo;
		ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (int i = 0; i < m_Fences.size(); ++i) 
		{
			VERIFYVULKANRESULT(vkCreateFence(m_Device, &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
		}
	}

	void DestroyFences()
	{
		for (int i = 0; i < m_Fences.size(); ++i)
		{
			vkDestroyFence(m_Device, m_Fences[i], VULKAN_CPU_ALLOCATOR);
		}
	}

	std::vector<std::string> m_CmdLine;
	std::string m_AssetsPath;

	std::vector<VkFence> m_Fences;
	VkDevice m_Device;
	std::shared_ptr<VulkanRHI> m_VulkanRHI;
	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndicesBuffer;
	UBOBuffer m_MVPBuffer;
	UBOData m_MVPData;
	VkDescriptorBufferInfo m_MVPDescriptor;
	VkDescriptorSetLayout m_DescriptorSetLayout;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_Pipeline;
	uint32 m_IndicesCount;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return new TriangleMode(800, 600, "Triangle", cmdLine);
}