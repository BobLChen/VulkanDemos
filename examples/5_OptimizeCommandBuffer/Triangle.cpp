#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DemoBase.h"
#include "Demo/DVKBuffer.h"
#include "Demo/DVKCommand.h"
#include "Demo/DVKUtils.h"
#include "Demo/DVKCamera.h"

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
			vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, &(m_VertexBuffer->buffer), offsets);
			vkCmdBindIndexBuffer(m_CommandBuffers[i], m_IndexBuffer->buffer, 0, VK_INDEX_TYPE_UINT16);
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
		writeDescriptorSet.pBufferInfo     = &(m_MVPBuffer->descriptor);
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
        shaderStages[0].module = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/2_Triangle/triangle.vert.spv");
		shaderStages[0].pName  = "main";
		shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/2_Triangle/triangle.frag.spv");
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

		m_MVPBuffer->CopyFrom(&m_MVPData, sizeof(UBOData));
	}

	void CreateUniformBuffers()
	{
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 0.1f, 1000.0f);
		m_ViewCamera.SetPosition(0, 0, -5.0f);
		m_ViewCamera.LookAt(0, 0, 0);

		m_MVPBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(UBOData),
			&m_MVPData
		);
		m_MVPBuffer->Map();
	}
	
	void DestroyUniformBuffers()
	{
		m_MVPBuffer->UnMap();
		delete m_MVPBuffer;
	}

	void CreateMeshBuffers()
	{
		std::vector<Vertex> vertices = {
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};
        
		std::vector<uint16> indices = { 0, 1, 2 };
		m_IndicesCount = (uint32)indices.size();
        
		// staging buffer
		vk_demo::DVKBuffer* vertStaging = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			vertices.size() * sizeof(Vertex), 
			vertices.data()
		);

		vk_demo::DVKBuffer* idexStaging = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			indices.size() * sizeof(uint16), 
			indices.data()
		);
		
		// reeal buffer
		m_VertexBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			vertices.size() * sizeof(Vertex)
		);

		m_IndexBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			indices.size() * sizeof(uint16)
		);

		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);
		cmdBuffer->Begin();

		VkBufferCopy copyRegion = {};
		copyRegion.size = vertices.size() * sizeof(Vertex);
		vkCmdCopyBuffer(cmdBuffer->cmdBuffer, vertStaging->buffer, m_VertexBuffer->buffer, 1, &copyRegion);

		copyRegion.size = indices.size() * sizeof(uint16);
		vkCmdCopyBuffer(cmdBuffer->cmdBuffer, idexStaging->buffer, m_IndexBuffer->buffer, 1, &copyRegion);
        
		cmdBuffer->End();
		cmdBuffer->Submit();
        
		delete cmdBuffer;
		delete vertStaging;
		delete idexStaging;
	}
    
	void DestroyMeshBuffers()
	{
		delete m_VertexBuffer;
		delete m_IndexBuffer;
	}
    
private:
	bool 							m_Ready = false;
	vk_demo::DVKCamera				m_ViewCamera;
    
	UBOData 						m_MVPData;

	vk_demo::DVKBuffer*				m_IndexBuffer = nullptr;
	vk_demo::DVKBuffer*				m_VertexBuffer = nullptr;
	vk_demo::DVKBuffer*				m_MVPBuffer = nullptr;

    VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
    
    VkPipeline 						m_Pipeline = VK_NULL_HANDLE;
	
	uint32 							m_IndicesCount = 0;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<TriangleModule>(1400, 900, "Command", cmdLine);
}
