#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
 
#include <vector>

class DynamicUniformBufferModule : public DemoBase
{
public:
	DynamicUniformBufferModule(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}
    
	virtual ~DynamicUniformBufferModule()
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

		LoadAssets();
		CreateGUI();
		CreateUniformBuffers();
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

		DestroyAssets();
		DestroyGUI();
        DestroyDescriptorSetLayout();
		DestroyPipelines();
		DestroyUniformBuffers();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

private:
    
	struct ModelBlock
	{
		Matrix4x4 model;
	};

	struct ColorBlock
	{
		Vector4 color;
	};

	struct ViewProjectionBlock
	{
		Matrix4x4 view;
		Matrix4x4 projection;
	};
    
	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		UpdateUniformBuffers(time, delta);
		
		DemoBase::Present(bufferIndex);
	}
    
	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();
        
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("DynamicUniformBuffer", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("Renderabls");
            
			ImGui::Checkbox("AutoRotate", &m_AutoRotate);
            
            uint32 alignment  = m_VulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
            uint32 colorAlign = Align(sizeof(ColorBlock), alignment);
            
			if (ImGui::SliderFloat("Alpha", &m_GlobalAlpha, 0.0f, 1.0f)) {
				for (int32 i = 0; i < m_Model->meshes.size(); ++i) {
                    ColorBlock* colorBlock = (ColorBlock*)(m_ColorDatas.data() + colorAlign * i);
					colorBlock->color.w = m_GlobalAlpha;
				}
			}
            
			ImGui::Combo("Select Mesh", &m_Selected, m_MeshNames.data(), m_MeshNames.size());
            
            ColorBlock* selectedColorBlock = (ColorBlock*)(m_ColorDatas.data() + m_Selected * colorAlign);
			ImGui::ColorEdit4("Mesh Color", (float*)&(selectedColorBlock->color), ImGuiColorEditFlags_AlphaBar);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
		}
        
		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
        
		if (m_GUI->Update()) {
			SetupCommandBuffers();
		}

		return hovered;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/Room/miniHouse_FBX.FBX",
			m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal }
		);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;
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
        
		uint32 alignment  = m_VulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
		uint32 modelAlign = Align(sizeof(ModelBlock), alignment);
		uint32 colorAlign = Align(sizeof(ColorBlock), alignment);

		for (int32 i = 0; i < m_CommandBuffers.size(); ++i)
		{
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            
            VkViewport viewport = {};
            viewport.x        = 0;
            viewport.y        = m_FrameHeight;
            viewport.width    = m_FrameWidth;
            viewport.height   = -(float)m_FrameHeight;    // flip y axis
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            
            VkRect2D scissor = {};
            scissor.extent.width  = m_FrameWidth;
            scissor.extent.height = m_FrameHeight;
            scissor.offset.x      = 0;
            scissor.offset.y      = 0;
            
            vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);
            
            vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->pipeline);
            
            for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex)
            {
				uint32 dynamicOffsets[2] = {
					meshIndex * modelAlign,
					meshIndex * colorAlign
				};
				vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->pipelineLayout, 0, 1, &m_DescriptorSet, 2, dynamicOffsets);
                m_Model->meshes[meshIndex]->BindDrawCmd(m_CommandBuffers[i]);
            }
			
			m_GUI->BindDrawCmd(m_CommandBuffers[i], m_RenderPass);

			vkCmdEndRenderPass(m_CommandBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
		}
	}
    
	void CreateDescriptorSet()
	{
		VkDescriptorPoolSize poolSizes[2];
		poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes[1].descriptorCount = 2;
        
		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 2;
		descriptorPoolInfo.pPoolSizes    = poolSizes;
		descriptorPoolInfo.maxSets       = 1;
		VERIFYVULKANRESULT(vkCreateDescriptorPool(m_Device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
        
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
        writeDescriptorSet.pBufferInfo     = &(m_ViewProjBuffer->descriptor);
        writeDescriptorSet.dstBinding      = 0;
        vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = m_ModelBuffer->buffer;
		bufferInfo.offset = 0;
		bufferInfo.range  = sizeof(ModelBlock);

		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        writeDescriptorSet.dstSet          = m_DescriptorSet;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writeDescriptorSet.pBufferInfo     = &bufferInfo;
        writeDescriptorSet.dstBinding      = 1;
        vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

		bufferInfo.buffer = m_ColorBuffer->buffer;
		bufferInfo.offset = 0;
		bufferInfo.range  = sizeof(ColorBlock);
		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        writeDescriptorSet.dstSet          = m_DescriptorSet;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        writeDescriptorSet.pBufferInfo     = &bufferInfo;
        writeDescriptorSet.dstBinding      = 2;
        vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
	}
    
	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();
		
		vk_demo::DVKGfxPipelineInfo pipelineInfo;
        pipelineInfo.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/13_DynamicUniformBuffer/obj.vert.spv");
		pipelineInfo.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/13_DynamicUniformBuffer/obj.frag.spv");
		
		pipelineInfo.blendAttachmentStates[0].blendEnable         = VK_TRUE;
		pipelineInfo.blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipelineInfo.blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipelineInfo.blendAttachmentStates[0].colorBlendOp        = VK_BLEND_OP_ADD;
		pipelineInfo.blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		pipelineInfo.blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		pipelineInfo.blendAttachmentStates[0].alphaBlendOp        = VK_BLEND_OP_ADD;

		m_Pipeline = vk_demo::DVKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);
		
		vkDestroyShaderModule(m_Device, pipelineInfo.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo.fragShaderModule, VULKAN_CPU_ALLOCATOR);
	}
    
	void DestroyPipelines()
	{
        delete m_Pipeline;
        m_Pipeline = nullptr;
	}
	
	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layoutBindings[3] = { };
		layoutBindings[0].binding 			 = 0;
		layoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount    = 1;
		layoutBindings[0].stageFlags 		 = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[0].pImmutableSamplers = nullptr;

		layoutBindings[1].binding 			 = 1;
		layoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		layoutBindings[1].descriptorCount    = 1;
		layoutBindings[1].stageFlags 		 = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[1].pImmutableSamplers = nullptr;

		layoutBindings[2].binding 			 = 2;
		layoutBindings[2].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		layoutBindings[2].descriptorCount    = 1;
		layoutBindings[2].stageFlags 		 = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[2].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 3;
		descSetLayoutInfo.pBindings    = layoutBindings;
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
		vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}
	
	void UpdateUniformBuffers(float time, float delta)
	{
        uint32 alignment  = m_VulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
        uint32 modelAlign = Align(sizeof(ModelBlock), alignment);
        
		if (m_AutoRotate)
        {
            for (int32 i = 0; i < m_Model->meshes.size(); ++i)
            {
                ModelBlock* modelBlock = (ModelBlock*)(m_ModelDatas.data() + modelAlign * i);
                modelBlock->model.AppendRotation(45.0f * delta, Vector3::UpVector);
            }
            m_ModelBuffer->CopyFrom(m_ModelDatas.data(), m_ModelBuffer->size);
        }

		m_ViewProjData.view = m_ViewCamera.GetView();
		m_ViewProjData.projection = m_ViewCamera.GetProjection();

		m_ViewProjBuffer->CopyFrom(&m_ViewProjData, sizeof(ViewProjectionBlock));

		m_ColorBuffer->CopyFrom(m_ColorDatas.data(), m_ColorBuffer->size);
	}

	void CreateUniformBuffers()
	{
		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
        Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 100.0f, 5000.0f);
		m_ViewCamera.SetPosition(boundCenter.x, boundCenter.y + 1000, boundCenter.z - boundSize.Size() * 1.5f);
		m_ViewCamera.LookAt(boundCenter);
        
		uint32 alignment  = m_VulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
        // world matrix dynamicbuffer
		uint32 modelAlign = Align(sizeof(ModelBlock), alignment);
        m_ModelDatas.resize(modelAlign * m_Model->meshes.size());
        for (int32 i = 0; i < m_Model->meshes.size(); ++i)
        {
            ModelBlock* modelBlock = (ModelBlock*)(m_ModelDatas.data() + modelAlign * i);
            modelBlock->model = m_Model->meshes[i]->linkNode->GetGlobalMatrix();
        }
        
		m_ModelBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			m_ModelDatas.size(),
			m_ModelDatas.data()
		);
		m_ModelBuffer->Map();
        
		m_ViewProjBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(ViewProjectionBlock),
			&(m_ViewProjData)
		);
		m_ViewProjBuffer->Map();
        
		// color per object
		uint32 colorAlign = Align(sizeof(ColorBlock), alignment);
		m_ColorDatas.resize(colorAlign * m_Model->meshes.size());
		for (int32 i = 0; i < m_Model->meshes.size(); ++i)
		{
            float r = MMath::RandRange(0.0f, 1.0f);
            float g = MMath::RandRange(0.0f, 1.0f);
            float b = MMath::RandRange(0.0f, 1.0f);
            ColorBlock* colorBlock = (ColorBlock*)(m_ColorDatas.data() + colorAlign * i);
			colorBlock->color.Set(r, g, b, 1.0f);
		}
		m_ColorBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			m_ColorDatas.size(),
			m_ColorDatas.data()
		);
		m_ColorBuffer->Map();
        
		for (int32 i = 0; i < m_Model->meshes.size(); ++i) {
			m_MeshNames.push_back(m_Model->meshes[i]->linkNode->name.c_str());
		}
	}
	
	void DestroyUniformBuffers()
	{
		m_ViewProjBuffer->UnMap();
		delete m_ViewProjBuffer;
		m_ViewProjBuffer = nullptr;

		m_ModelBuffer->UnMap();
		delete m_ModelBuffer;
		m_ModelBuffer = nullptr;

		m_ColorBuffer->UnMap();
		delete m_ColorBuffer;
		m_ColorBuffer = nullptr;
	}
    
	void CreateGUI()
	{
		m_GUI = new ImageGUIContext();
		m_GUI->Init("assets/fonts/Ubuntu-Regular.ttf");
	}

	void DestroyGUI()
	{
		m_GUI->Destroy();
		delete m_GUI;
	}

private:

	bool							m_AutoRotate = false;
	bool 							m_Ready = false;
    
	vk_demo::DVKCamera				m_ViewCamera;

    std::vector<uint8>              m_ModelDatas;
	vk_demo::DVKBuffer*				m_ModelBuffer = nullptr;

	vk_demo::DVKBuffer*				m_ViewProjBuffer = nullptr;
	ViewProjectionBlock				m_ViewProjData;

	std::vector<uint8>			    m_ColorDatas;
	vk_demo::DVKBuffer*				m_ColorBuffer = nullptr;

	std::vector<const char*>		m_MeshNames;
	int32							m_Selected = 0;
	float							m_GlobalAlpha = 1.0f;

    vk_demo::DVKGfxPipeline*        m_Pipeline = nullptr;

	vk_demo::DVKModel*				m_Model = nullptr;

    VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet = VK_NULL_HANDLE;
    
	ImageGUIContext*				m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<DynamicUniformBufferModule>(1400, 900, "DynamicUniformBuffer", cmdLine);
}
