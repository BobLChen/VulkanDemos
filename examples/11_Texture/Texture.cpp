#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"
#include "Demo/DVKCamera.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class TextureModule : public DemoBase
{
public:
	TextureModule(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}
    
	virtual ~TextureModule()
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
		Vector3 lightDir;
		float curvature;

		Vector3 lightColor;
		float exposure;

		Vector2 curvatureScaleBias;
		float blurredLevel;
		float padding;
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
            ImGui::Begin("Texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("SSS Demo");

			for (int32 i = 0; i < m_Model->meshes.size(); ++i)
			{
				vk_demo::DVKMesh* mesh = m_Model->meshes[i];
				ImGui::Text("%-20s Tri:%d", mesh->linkNode->name.c_str(), mesh->triangleCount);
			}

			ImGui::SliderFloat("Curvature", &(m_ParamData.curvature),       0.0f, 10.0f);
			ImGui::SliderFloat2("CurvatureBias", (float*)&(m_ParamData.curvatureScaleBias), 0.0f, 1.0f);

			ImGui::SliderFloat("BlurredLevel", &(m_ParamData.blurredLevel), 0.0f, 12.0f);
			ImGui::SliderFloat("Exposure", &(m_ParamData.exposure),         0.0f, 10.0f);

			ImGui::SliderFloat3("LightDirection", (float*)&(m_ParamData.lightDir), -10.0f, 10.0f);
			ImGui::ColorEdit3("LightColor", (float*)&(m_ParamData.lightColor));

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
			"assets/models/head.obj",
			m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal, VertexAttribute::VA_Tangent }
		);

		m_TexDiffuse       = vk_demo::DVKTexture::Create2D("assets/textures/head_diffuse.jpg", m_VulkanDevice, cmdBuffer);
		m_TexNormal        = vk_demo::DVKTexture::Create2D("assets/textures/head_normal.jpg", m_VulkanDevice, cmdBuffer);
		m_TexCurvature     = vk_demo::DVKTexture::Create2D("assets/textures/curvatureLUT.png", m_VulkanDevice, cmdBuffer);
		m_TexPreIntegrated = vk_demo::DVKTexture::Create2D("assets/textures/preIntegratedLUT.png", m_VulkanDevice, cmdBuffer);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;

		delete m_TexDiffuse;
		delete m_TexNormal;
		delete m_TexCurvature;
		delete m_TexPreIntegrated;
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
            vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->pipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
            
            for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
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
		poolSizes[0].descriptorCount = 2;
		poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 4;
        
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
        writeDescriptorSet.pBufferInfo     = &(m_MVPBuffer->descriptor);
        writeDescriptorSet.dstBinding      = 0;
        vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
        
        ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        writeDescriptorSet.dstSet          = m_DescriptorSet;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet.pBufferInfo     = &(m_ParamBuffer->descriptor);
        writeDescriptorSet.dstBinding      = 1;
        vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

		std::vector<vk_demo::DVKTexture*> textures = { m_TexDiffuse, m_TexNormal, m_TexCurvature, m_TexPreIntegrated };
		for (int32 i = 0; i < 4; ++i)
		{
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet          = m_DescriptorSet;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.pBufferInfo     = nullptr;
			writeDescriptorSet.pImageInfo      = &(textures[i]->descriptorInfo);
			writeDescriptorSet.dstBinding      = 2 + i;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
		}
	}
    
	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();
		
		vk_demo::DVKGfxPipelineInfo pipelineInfo;
        pipelineInfo.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/11_Texture/texture.vert.spv");
		pipelineInfo.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/11_Texture/texture.frag.spv");
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
		VkDescriptorSetLayoutBinding layoutBindings[6] = { };
		layoutBindings[0].binding 			 = 0;
		layoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount    = 1;
		layoutBindings[0].stageFlags 		 = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[0].pImmutableSamplers = nullptr;

		layoutBindings[1].binding 			 = 1;
		layoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[1].descriptorCount    = 1;
		layoutBindings[1].stageFlags 		 = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = nullptr;

		for (int32 i = 0; i < 4; ++i)
		{
			layoutBindings[2 + i].binding 			 = 2 + i;
			layoutBindings[2 + i].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBindings[2 + i].descriptorCount    = 1;
			layoutBindings[2 + i].stageFlags 		 = VK_SHADER_STAGE_FRAGMENT_BIT;
			layoutBindings[2 + i].pImmutableSamplers = nullptr;
		}
		
		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 6;
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
		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();

		m_MVPBuffer->CopyFrom(&m_MVPData, sizeof(MVPBlock));
		m_ParamBuffer->CopyFrom(&m_ParamData, sizeof(ParamBlock));
	}
    
	void CreateUniformBuffers()
	{
		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
        Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		m_MVPData.model.AppendRotation(180, Vector3::UpVector);

		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 0.1f, 1000.0f);
		m_ViewCamera.SetPosition(boundCenter.x, boundCenter.y, boundCenter.z - boundSize.Size() * 1.0f);
		m_ViewCamera.LookAt(boundCenter);
		
		m_MVPBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(MVPBlock),
			&(m_MVPData)
		);
		m_MVPBuffer->Map();

		m_ParamData.blurredLevel = 2.0;
		m_ParamData.curvature = 3.5;
		m_ParamData.curvatureScaleBias.x = 0.101;
		m_ParamData.curvatureScaleBias.y = -0.001;
		m_ParamData.exposure = 1.0;
		m_ParamData.lightColor.Set(240.0f / 255.0f, 200.0f / 255.0f, 166.0f / 255.0f);
		m_ParamData.lightDir.Set(1, 0, -1.0);
		m_ParamData.lightDir.Normalize();
		m_ParamData.padding = 0.0;
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
		m_MVPBuffer->UnMap();
		delete m_MVPBuffer;
		m_MVPBuffer = nullptr;

		m_ParamBuffer->UnMap();
		delete m_ParamBuffer;
		m_ParamBuffer = nullptr;
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

	bool 							m_Ready = false;
    
	vk_demo::DVKCamera				m_ViewCamera;

	MVPBlock 						m_MVPData;
	vk_demo::DVKBuffer*				m_MVPBuffer;

	ParamBlock						m_ParamData;
	vk_demo::DVKBuffer*				m_ParamBuffer = nullptr;

	vk_demo::DVKTexture*			m_TexDiffuse = nullptr;
	vk_demo::DVKTexture*			m_TexNormal = nullptr;
	vk_demo::DVKTexture*			m_TexCurvature = nullptr;
	vk_demo::DVKTexture*			m_TexPreIntegrated = nullptr;
	
    vk_demo::DVKGfxPipeline*           m_Pipeline = nullptr;

	vk_demo::DVKModel*				m_Model = nullptr;

    VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet = VK_NULL_HANDLE;
    
	ImageGUIContext*				m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<TextureModule>(1400, 900, "Texture", cmdLine);
}
