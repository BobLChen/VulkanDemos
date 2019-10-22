#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DemoBase.h"
#include "Demo/DVKCamera.h"
#include "Demo/FileManager.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class TriangleModule : public DemoBase
{
public:
	TriangleModule(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}
    
	virtual ~TriangleModule()
	{

	}

	virtual bool PreInit() override
	{
		return true;
	}

	virtual bool Init() override
	{
		DemoBase::Setup();
		DemoBase::Prepare();

		CreateMeshBuffers();
		CreateUniformBuffers();
        CreateDescriptorPool();
		CreateDescriptorSetLayout();
        CreateDescriptorSet();
		CreatePipelines();
		SetupCommandBuffers();

		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
		DemoBase::Release();

        DestroyDescriptorSetLayout();
		DestroyDescriptorPool();
		DestroyPipelines();
		DestroyUniformBuffers();
		DestroyMeshBuffers();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

private:

	struct GPUBuffer
	{
		VkDeviceMemory 	memory;
		VkBuffer 		buffer;

		GPUBuffer()
			: memory(VK_NULL_HANDLE)
			, buffer(VK_NULL_HANDLE)
		{

		}
	};
	
	typedef GPUBuffer 	IndexBuffer;
	typedef GPUBuffer 	VertexBuffer;
	typedef GPUBuffer 	UBOBuffer;

	struct Vertex
	{
		float position[3];
		float color[3];
	};

	struct UBOData
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	VkShaderModule LoadSPIPVShader(const std::string& filepath)
	{
		uint8* dataPtr  = nullptr;
		uint32 dataSize = 0;
		FileManager::ReadFile(filepath, dataPtr, dataSize);

		VkShaderModuleCreateInfo moduleCreateInfo;
		ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
		moduleCreateInfo.codeSize = dataSize;
		moduleCreateInfo.pCode    = (uint32_t*)dataPtr;

		VkShaderModule shaderModule;
		VERIFYVULKANRESULT(vkCreateShaderModule(m_Device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
		delete[] dataPtr;
		
		return shaderModule;
	}
    
	void Draw(float time, float delta)
	{
		m_ViewCamera.Update(time, delta);
		UpdateUniformBuffers(time, delta);
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();
        DemoBase::Present(bufferIndex);
	}

	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color        = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = m_RenderPass;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues    = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
        renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;
        
		for (int32 i = 0; i < m_CommandBuffers.size(); ++i)
		{
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];
            
			VkViewport viewport = {};
			viewport.x        = 0;
			viewport.y        = m_FrameHeight;
            viewport.width    = (float)m_FrameWidth;
            viewport.height   = -(float)m_FrameHeight;    // flip y axis
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
            
			VkRect2D scissor = {};
            scissor.extent.width  = m_FrameWidth;
            scissor.extent.height = m_FrameHeight;
			scissor.offset.x      = 0;
			scissor.offset.y      = 0;
            
			VkDeviceSize offsets[1] = { 0 };
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBeginInfo));
            
			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);

			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
			vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
			vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, &m_VertexBuffer.buffer, offsets);
			vkCmdBindIndexBuffer(m_CommandBuffers[i], m_IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(m_CommandBuffers[i], m_IndicesCount, 1, 0, 0, 0);
			
			vkCmdEndRenderPass(m_CommandBuffers[i]);
            
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
		}
	}
    
	void CreateDescriptorSet()
	{
		VkDescriptorSetAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
		allocInfo.descriptorPool     = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts        = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_DescriptorSet));
        
		VkWriteDescriptorSet writeDescriptorSet;
		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet          = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo     = &m_MVPDescriptor;
		writeDescriptorSet.dstBinding      = 0;
		vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
	}
    
	void CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 1;
        
		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 1;
		descriptorPoolInfo.pPoolSizes    = &poolSize;
		descriptorPoolInfo.maxSets       = 1;
		VERIFYVULKANRESULT(vkCreateDescriptorPool(m_Device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
	}
    
	void DestroyDescriptorPool()
	{
		vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}
    
	void CreatePipelines()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
		VkPipelineRasterizationStateCreateInfo rasterizationState;
		ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
		rasterizationState.polygonMode 			   = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode                = VK_CULL_MODE_NONE;
		rasterizationState.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable        = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable         = VK_FALSE;
		rasterizationState.lineWidth 			   = 1.0f;
        
		VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
        blendAttachmentState[0].colorWriteMask = (
            VK_COLOR_COMPONENT_R_BIT |
            VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT
        );
		blendAttachmentState[0].blendEnable = VK_FALSE;
        
		VkPipelineColorBlendStateCreateInfo colorBlendState;
		ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments    = blendAttachmentState;
        
		VkPipelineViewportStateCreateInfo viewportState;
		ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
		viewportState.viewportCount = 1;
		viewportState.scissorCount  = 1;
        
		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
		VkPipelineDynamicStateCreateInfo dynamicState;
		ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
		dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();
		dynamicState.pDynamicStates    = dynamicStateEnables.data();
        
		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
		depthStencilState.depthTestEnable 		= VK_TRUE;
		depthStencilState.depthWriteEnable 		= VK_TRUE;
		depthStencilState.depthCompareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp 			= VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp 			= VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp 		= VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable 	= VK_FALSE;
		depthStencilState.front 				= depthStencilState.back;

		VkPipelineMultisampleStateCreateInfo multisampleState;
		ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
		multisampleState.rasterizationSamples = m_SampleCount;
		multisampleState.pSampleMask 		  = nullptr;
		
		// (triangle.vert):
		// layout (location = 0) in vec3 inPos;
		// layout (location = 1) in vec3 inColor;
		// Attribute location 0: Position
		// Attribute location 1: Color
		// vertex input bindding
		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding   = 0; // Vertex Buffer 0
		vertexInputBinding.stride    = sizeof(Vertex); // Position + Color
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(2);
		// position
		vertexInputAttributs[0].binding  = 0;
        vertexInputAttributs[0].location = 0; // triangle.vert : layout (location = 0)
		vertexInputAttributs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[0].offset   = 0;
		// color
		vertexInputAttributs[1].binding  = 0;
		vertexInputAttributs[1].location = 1; // triangle.vert : layout (location = 1)
		vertexInputAttributs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[1].offset   = 12;
		
		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount   = 1;
		vertexInputState.pVertexBindingDescriptions      = &vertexInputBinding;
		vertexInputState.vertexAttributeDescriptionCount = 2;
		vertexInputState.pVertexAttributeDescriptions    = vertexInputAttributs.data();
        
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
		ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = LoadSPIPVShader("assets/shaders/2_Triangle/triangle.vert.spv");
		shaderStages[0].pName  = "main";
		shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = LoadSPIPVShader("assets/shaders/2_Triangle/triangle.frag.spv");
		shaderStages[1].pName  = "main";
        
		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout 				= m_PipelineLayout;
		pipelineCreateInfo.renderPass 			= m_RenderPass;
		pipelineCreateInfo.stageCount 			= (uint32_t)shaderStages.size();
		pipelineCreateInfo.pStages 				= shaderStages.data();
		pipelineCreateInfo.pVertexInputState 	= &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState 	= &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState 	= &rasterizationState;
		pipelineCreateInfo.pColorBlendState 	= &colorBlendState;
		pipelineCreateInfo.pMultisampleState 	= &multisampleState;
		pipelineCreateInfo.pViewportState 		= &viewportState;
		pipelineCreateInfo.pDepthStencilState 	= &depthStencilState;
		pipelineCreateInfo.pDynamicState 		= &dynamicState;
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(m_Device, m_PipelineCache, 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));
		
		vkDestroyShaderModule(m_Device, shaderStages[0].module, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, shaderStages[1].module, VULKAN_CPU_ALLOCATOR);
	}
    
	void DestroyPipelines()
	{
		vkDestroyPipeline(m_Device, m_Pipeline, VULKAN_CPU_ALLOCATOR);
	}
	
	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding 			 = 0;
		layoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount    = 1;
		layoutBinding.stageFlags 		 = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.pImmutableSamplers = nullptr;
        
		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 1;
		descSetLayoutInfo.pBindings    = &layoutBinding;
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(m_Device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));
        
		VkPipelineLayoutCreateInfo pipeLayoutInfo;
		ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipeLayoutInfo.setLayoutCount = 1;
		pipeLayoutInfo.pSetLayouts    = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkCreatePipelineLayout(m_Device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
	}
    
	void DestroyDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
	}
	
	void UpdateUniformBuffers(float time, float delta)
	{
		m_MVPData.model.AppendRotation(90.0f * delta, Vector3::UpVector);
		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();

		uint8_t *pData = nullptr;
		VERIFYVULKANRESULT(vkMapMemory(m_Device, m_MVPBuffer.memory, 0, sizeof(UBOData), 0, (void**)&pData));
		std::memcpy(pData, &m_MVPData, sizeof(UBOData));
		vkUnmapMemory(m_Device, m_MVPBuffer.memory);
	}

	void CreateUniformBuffers()
	{
		VkBufferCreateInfo bufferInfo;
		ZeroVulkanStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		bufferInfo.size  = sizeof(UBOData);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &bufferInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.buffer));
        
		VkMemoryRequirements memReqInfo;
		vkGetBufferMemoryRequirements(m_Device, m_MVPBuffer.buffer, &memReqInfo);
		
		VkMemoryAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		allocInfo.allocationSize  = memReqInfo.size;
		allocInfo.memoryTypeIndex = GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &allocInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_MVPBuffer.buffer, m_MVPBuffer.memory, 0));
        
		m_MVPDescriptor.buffer = m_MVPBuffer.buffer;
		m_MVPDescriptor.offset = 0;
		m_MVPDescriptor.range  = sizeof(UBOData);
        
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 0.1f, 1000.0f);
		m_ViewCamera.SetPosition(0, 0, -5.0f);
		m_ViewCamera.LookAt(0, 0, 0);
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
		m_IndicesCount = (uint32)indices.size();
        
		// 顶点数据以及索引数据在整个生命周期中几乎不会发生改变，因此最佳的方式是将这些数据存储到GPU的内存中。
		// 存储到GPU内存也能加快GPU的访问。为了存储到GPU内存中，需要如下几个步骤。
		// 1、在主机端(Host)创建一个Buffer
		// 2、将数据拷贝至该Buffer
		// 3、在GPU端(Local Device)创建一个Buffer
		// 4、通过Transfer簇将数据从主机端拷贝至GPU端
		// 5、删除主基端(Host)的Buffer
		// 6、使用GPU端(Local Device)的Buffer进行渲染
		VertexBuffer tempVertexBuffer;
		IndexBuffer  tempIndexBuffer;
        
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
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, tempVertexBuffer.buffer, tempVertexBuffer.memory, 0));
        
		VERIFYVULKANRESULT(vkMapMemory(m_Device, tempVertexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, vertices.data(), vertexBufferInfo.size);
		vkUnmapMemory(m_Device, tempVertexBuffer.memory);
        
		// local device vertex buffer
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.buffer));
        
		vkGetBufferMemoryRequirements(m_Device, m_VertexBuffer.buffer, &memReqInfo);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_VertexBuffer.buffer, m_VertexBuffer.memory, 0));
        
		// index buffer
		VkBufferCreateInfo indexBufferInfo;
		ZeroVulkanStruct(indexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		indexBufferInfo.size  = m_IndicesCount * sizeof(uint16);
		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.buffer));
        
		vkGetBufferMemoryRequirements(m_Device, tempIndexBuffer.buffer, &memReqInfo);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, tempIndexBuffer.buffer, tempIndexBuffer.memory, 0));
        
		VERIFYVULKANRESULT(vkMapMemory(m_Device, tempIndexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, indices.data(), indexBufferInfo.size);
		vkUnmapMemory(m_Device, tempIndexBuffer.memory);
		
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_IndexBuffer.buffer));
		
		vkGetBufferMemoryRequirements(m_Device, m_IndexBuffer.buffer, &memReqInfo);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_IndexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_IndexBuffer.buffer, m_IndexBuffer.memory, 0));
        
		VkCommandBuffer xferCmdBuffer;
		// gfx queue自带transfer功能，为了优化需要使用专有的xfer queue。这里为了简单，先将就用。
		VkCommandBufferAllocateInfo xferCmdBufferInfo;
		ZeroVulkanStruct(xferCmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
        xferCmdBufferInfo.commandPool        = m_CommandPool;
		xferCmdBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
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
		vkCmdCopyBuffer(xferCmdBuffer, tempIndexBuffer.buffer, m_IndexBuffer.buffer, 1, &copyRegion);
        
		// 结束录制
		VERIFYVULKANRESULT(vkEndCommandBuffer(xferCmdBuffer));
		
		// 提交命令，并且等待命令执行完毕。
		VkSubmitInfo submitInfo;
		ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &xferCmdBuffer;

		VkFenceCreateInfo fenceInfo;
		ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceInfo.flags = 0;
        
		VkFence fence = VK_NULL_HANDLE;
		VERIFYVULKANRESULT(vkCreateFence(m_Device, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));
		VERIFYVULKANRESULT(vkQueueSubmit(m_GfxQueue, 1, &submitInfo, fence));
		VERIFYVULKANRESULT(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, MAX_int64));
        
		vkDestroyFence(m_Device, fence, VULKAN_CPU_ALLOCATOR);
		vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &xferCmdBuffer);
        
		vkDestroyBuffer(m_Device, tempVertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, tempVertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(m_Device, tempIndexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, tempIndexBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}
    
	void DestroyMeshBuffers()
	{
		vkDestroyBuffer(m_Device, m_VertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, m_VertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(m_Device, m_IndexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, m_IndexBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}
    
private:
	bool 							m_Ready = false;
	vk_demo::DVKCamera				m_ViewCamera;
    
	UBOData 						m_MVPData;

	VertexBuffer 					m_VertexBuffer;
	IndexBuffer 					m_IndexBuffer;
	UBOBuffer 						m_MVPBuffer;

	VkDescriptorBufferInfo 			m_MVPDescriptor;
	
    VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
    
    VkPipeline 						m_Pipeline = VK_NULL_HANDLE;
	
	uint32 							m_IndicesCount = 0;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<TriangleModule>(1400, 900, "DemoBase", cmdLine);
}