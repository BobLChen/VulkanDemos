#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "File/FileManager.h"
#include "UI/ImageGUIContext.h"

#include <vector>
#include <fstream>

class PipelinesModule : public DemoBase
{
public:
	PipelinesModule(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}
    
	virtual ~PipelinesModule()
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
    
	struct MVPBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	struct ParamBlock
	{
		float intensity;
	};
    
	void Draw(float time, float delta)
	{
        UpdateUI(time, delta);
		UpdateUniformBuffers(time, delta);
        DemoBase::Present();
	}
    
	void UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();
        
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Pipelines!", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			
			ImGui::Checkbox("AutoRotate", &m_AutoRotate);
			
			ImGui::SliderFloat("Intensity", &(m_ParamData.intensity), 0.0f, 10.0f);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
		}
        
		m_GUI->EndFrame();
        
		if (m_GUI->Update()) {
			SetupCommandBuffers();
		}
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/Vela_Template.fbx",
			m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_Normal }
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
        
		for (int32 i = 0; i < m_CommandBuffers.size(); ++i)
		{
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			for (int32 j = 0; j < 3; ++j)
			{
				int32 ww = 1.0f / 3 * m_FrameWidth;
				int32 tx = j * ww;

				VkViewport viewport = {};
				viewport.x        = tx;
				viewport.y        = m_FrameHeight;
				viewport.width    = ww;
				viewport.height   = -(float)m_FrameHeight;    // flip y axis
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				
				VkRect2D scissor = {};
				scissor.extent.width  = ww;
				scissor.extent.height = m_FrameHeight;
				scissor.offset.x      = tx;
				scissor.offset.y      = 0;

				vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
				vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);

				vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines[j]->pipeline);
                vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines[j]->pipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);
                
				for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
					m_Model->meshes[meshIndex]->BindDrawCmd(m_CommandBuffers[i]);
				}
			}
			
			m_GUI->BindDrawCmd(m_CommandBuffers[i], m_RenderPass);

			vkCmdEndRenderPass(m_CommandBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
		}
	}
    
	void CreateDescriptorSet()
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 2;
        
		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 1;
		descriptorPoolInfo.pPoolSizes    = &poolSize;
		descriptorPoolInfo.maxSets       = m_Model->meshes.size();
		VERIFYVULKANRESULT(vkCreateDescriptorPool(m_Device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));

		m_DescriptorSets.resize(m_Model->meshes.size());
		for (int32 i = 0; i < m_DescriptorSets.size(); ++i)
		{
			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

			VkDescriptorSetAllocateInfo allocInfo;
			ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
			allocInfo.descriptorPool     = m_DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts        = &m_DescriptorSetLayout;
			VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_Device, &allocInfo, &descriptorSet));
        
			VkWriteDescriptorSet writeDescriptorSet;
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet          = descriptorSet;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.pBufferInfo     = &(m_MVPBuffers[i]->descriptor);
			writeDescriptorSet.dstBinding      = 0;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet          = descriptorSet;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.pBufferInfo     = &(m_ParamBuffer->descriptor);
			writeDescriptorSet.dstBinding      = 1;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

			m_DescriptorSets[i] = descriptorSet;
		}
	}
    
	void CreatePipelines()
	{
		m_Pipelines.resize(3);

		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();
		
		vk_demo::DVKPipelineInfo pipelineInfo0;
        pipelineInfo0.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/10_Pipelines/pipeline0.vert.spv");
		pipelineInfo0.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/10_Pipelines/pipeline0.frag.spv");
		m_Pipelines[0] = vk_demo::DVKPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo0, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);
		
		vk_demo::DVKPipelineInfo pipelineInfo1;
        pipelineInfo1.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/10_Pipelines/pipeline1.vert.spv");
		pipelineInfo1.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/10_Pipelines/pipeline1.frag.spv");
		m_Pipelines[1] = vk_demo::DVKPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo1, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);
	
		vk_demo::DVKPipelineInfo pipelineInfo2;
		pipelineInfo2.rasterizationState.polygonMode = VkPolygonMode::VK_POLYGON_MODE_LINE;
        pipelineInfo2.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/10_Pipelines/pipeline2.vert.spv");
		pipelineInfo2.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/10_Pipelines/pipeline2.frag.spv");
		m_Pipelines[2] = vk_demo::DVKPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo2, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);
		
		vkDestroyShaderModule(m_Device, pipelineInfo0.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo0.fragShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo1.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo1.fragShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo2.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo2.fragShaderModule, VULKAN_CPU_ALLOCATOR);
	}
    
	void DestroyPipelines()
	{
		for (int32 i = 0; i < m_Pipelines.size(); ++i) {
			delete m_Pipelines[i];
		}
		m_Pipelines.clear();
	}
	
	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layoutBindings[2] = { };
		layoutBindings[0].binding 			 = 0;
		layoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount    = 1;
		layoutBindings[0].stageFlags 		 = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[0].pImmutableSamplers = nullptr;

		layoutBindings[1].binding 			 = 1;
		layoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[1].descriptorCount    = 1;
		layoutBindings[1].stageFlags 		 = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[1].pImmutableSamplers = nullptr;
        
		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 2;
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
		if (m_AutoRotate) {
			for (int32 i = 0; i < m_MVPDatas.size(); ++i) {
				m_MVPDatas[i].model.AppendRotation(90.0f * delta, Vector3::UpVector);
				m_MVPBuffers[i]->CopyFrom(&(m_MVPDatas[i]), sizeof(MVPBlock));
			}
		}
		m_ParamBuffer->CopyFrom(&m_ParamData, sizeof(ParamBlock));
	}

	void CreateUniformBuffers()
	{
		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
        Vector3 boundCenter = bounds.min + boundSize * 0.5f;
		boundCenter.z -= boundSize.Size();

		m_MVPDatas.resize(m_Model->meshes.size());
		m_MVPBuffers.resize(m_Model->meshes.size());

		for (int32 i = 0; i < m_Model->meshes.size(); ++i)
		{
			m_MVPDatas[i].model.SetIdentity();
			m_MVPDatas[i].model.SetOrigin(Vector3(0, 0, 0));
        
			m_MVPDatas[i].view.SetIdentity();
			m_MVPDatas[i].view.SetOrigin(boundCenter);
			m_MVPDatas[i].view.SetInverse();

			m_MVPDatas[i].projection.SetIdentity();
			m_MVPDatas[i].projection.Perspective(MMath::DegreesToRadians(75.0f), (float)GetWidth() / 3.0f, (float)GetHeight(), 0.01f, 3000.0f);
		
			m_MVPBuffers[i] = vk_demo::DVKBuffer::CreateBuffer(
				m_VulkanDevice, 
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				sizeof(MVPBlock),
				&(m_MVPDatas[i])
			);
			m_MVPBuffers[i]->Map();
		}

		m_ParamData.intensity = 5.0f;
		m_ParamBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(ParamBlock),
			&(m_ParamData)
		);
		m_ParamBuffer->Map();
	}
	
	void DestroyUniformBuffers()
	{
		m_MVPDatas.clear();

		for (int32 i = 0; i < m_MVPBuffers.size(); ++i) {
			vk_demo::DVKBuffer* buffer = m_MVPBuffers[i];
			buffer->UnMap();
			delete buffer;
		}

		m_MVPBuffers.clear();

		m_ParamBuffer->UnMap();
		delete m_ParamBuffer;
		m_ParamBuffer = nullptr;
	}

	void CreateGUI()
	{
		m_GUI = new ImageGUIContext();
		m_GUI->Init("assets/fonts/Roboto-Medium.ttf");
	}

	void DestroyGUI()
	{
		m_GUI->Destroy();
		delete m_GUI;
	}

private:
	typedef std::vector<vk_demo::DVKBuffer*>		DVKBuffers;
	typedef std::vector<VkDescriptorSet>			VkDescriptorSets;
	typedef std::vector<vk_demo::DVKPipeline*>		DVKPipelines;

	bool							m_AutoRotate = false;
	bool 							m_Ready = false;
    
	std::vector<MVPBlock> 			m_MVPDatas;
	DVKBuffers						m_MVPBuffers;

	ParamBlock						m_ParamData;
	vk_demo::DVKBuffer*				m_ParamBuffer = nullptr;
	
	DVKPipelines					m_Pipelines;

	vk_demo::DVKModel*				m_Model = nullptr;

    VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSets 				m_DescriptorSets;
    
	ImageGUIContext*				m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<PipelinesModule>(1400, 900, "Pipelines", cmdLine);
}
