#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"

#include <vector>

class InputAttachmentsDemo : public DemoBase
{
public:
	InputAttachmentsDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}
    
	virtual ~InputAttachmentsDemo()
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
		DestroyPipelines();
		DestroyUniformBuffers();

		DestroyAttachments();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

protected:

	void CreateFrameBuffers() override
	{
		DestroyFrameBuffers();
		
		int32 fwidth    = GetVulkanRHI()->GetSwapChain()->GetWidth();
		int32 fheight   = GetVulkanRHI()->GetSwapChain()->GetHeight();
		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();

		VkImageView attachments[4];

		VkFramebufferCreateInfo frameBufferCreateInfo;
		ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
		frameBufferCreateInfo.renderPass      = m_RenderPass;
		frameBufferCreateInfo.attachmentCount = 4;
		frameBufferCreateInfo.pAttachments    = attachments;
		frameBufferCreateInfo.width			  = fwidth;
		frameBufferCreateInfo.height		  = fheight;
		frameBufferCreateInfo.layers		  = 1;

		const std::vector<VkImageView>& backbufferViews = GetVulkanRHI()->GetBackbufferViews();

		m_FrameBuffers.resize(backbufferViews.size());
		for (uint32 i = 0; i < m_FrameBuffers.size(); ++i) {
			attachments[0] = backbufferViews[i];
			attachments[1] = m_AttachsColor[i]->imageView;
            attachments[2] = m_AttachsNormal[i]->imageView;
			attachments[3] = m_AttachsDepth[i]->imageView;
			VERIFYVULKANRESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
		}
	}

	void CreateDepthStencil() override
	{

	}

	void DestoryDepthStencil() override
	{
		
	}

	void DestroyAttachments()
	{
		for (int32 i = 0; i < m_AttachsDepth.size(); ++i) 
		{
			vk_demo::DVKTexture* texture = m_AttachsDepth[i];
			delete texture;
		}
		m_AttachsDepth.clear();

		for (int32 i = 0; i < m_AttachsColor.size(); ++i) 
		{
			vk_demo::DVKTexture* texture = m_AttachsColor[i];
			delete texture;
		}
		m_AttachsColor.clear();
        
        for (int32 i = 0; i < m_AttachsNormal.size(); ++i)
        {
            vk_demo::DVKTexture* texture = m_AttachsNormal[i];
            delete texture;
        }
        m_AttachsNormal.clear();
	}

	void CreateAttachments()
	{
		auto swapChain  = GetVulkanRHI()->GetSwapChain();
		int32 fwidth    = swapChain->GetWidth();
		int32 fheight   = swapChain->GetHeight();
		int32 numBuffer = swapChain->GetBackBufferCount();
		
		m_AttachsColor.resize(numBuffer);
        m_AttachsNormal.resize(numBuffer);
		m_AttachsDepth.resize(numBuffer);
        
		for (int32 i = 0; i < m_AttachsColor.size(); ++i)
		{
			m_AttachsColor[i] = vk_demo::DVKTexture::CreateAttachment(
				m_VulkanDevice,
				PixelFormatToVkFormat(GetVulkanRHI()->GetPixelFormat(), false), 
				VK_IMAGE_ASPECT_COLOR_BIT,
				fwidth, fheight,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			);
		}
        
        for (int32 i = 0; i < m_AttachsNormal.size(); ++i)
        {
            m_AttachsNormal[i] = vk_demo::DVKTexture::CreateAttachment(
                m_VulkanDevice,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_ASPECT_COLOR_BIT,
                fwidth, fheight,
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
            );
        }
        
		for (int32 i = 0; i < m_AttachsDepth.size(); ++i)
		{
			m_AttachsDepth[i] = vk_demo::DVKTexture::CreateAttachment(
				m_VulkanDevice,
				PixelFormatToVkFormat(m_DepthFormat, false), 
				VK_IMAGE_ASPECT_DEPTH_BIT,
				fwidth, fheight,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			);
		}
	}

	void CreateRenderPass() override
	{
		DestoryRenderPass();
		DestroyAttachments();
		CreateAttachments();

		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		PixelFormat pixelFormat = GetVulkanRHI()->GetPixelFormat();

		std::vector<VkAttachmentDescription> attachments(4);
		// swap chain attachment
		attachments[0].format		  = PixelFormatToVkFormat(pixelFormat, false);
		attachments[0].samples		  = m_SampleCount;
		attachments[0].loadOp		  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout	  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// color attachment
		attachments[1].format         = PixelFormatToVkFormat(pixelFormat, false);
		attachments[1].samples        = m_SampleCount;
		attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        // normal attachment
        attachments[2].format         = VK_FORMAT_R8G8B8A8_UNORM;
        attachments[2].samples        = m_SampleCount;
        attachments[2].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// depth stencil attachment
		attachments[3].format         = PixelFormatToVkFormat(m_DepthFormat, false);
		attachments[3].samples        = m_SampleCount;
		attachments[3].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[3].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[3].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[3].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[3].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		VkAttachmentReference colorReferences[2];
		colorReferences[0].attachment = 1;
		colorReferences[0].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorReferences[1].attachment = 2;
        colorReferences[1].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkAttachmentReference swapReference = { };
		swapReference.attachment = 0;
		swapReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference = { };
		depthReference.attachment = 3;
		depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
		VkAttachmentReference inputReferences[3];
		inputReferences[0].attachment = 1;
		inputReferences[0].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        inputReferences[1].attachment = 2;
        inputReferences[1].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		inputReferences[2].attachment = 3;
		inputReferences[2].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
		std::vector<VkSubpassDescription> subpassDescriptions(2);
		subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[0].colorAttachmentCount    = 2;
		subpassDescriptions[0].pColorAttachments       = colorReferences;
		subpassDescriptions[0].pDepthStencilAttachment = &depthReference;
        
		subpassDescriptions[1].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[1].colorAttachmentCount    = 1;
		subpassDescriptions[1].pColorAttachments       = &swapReference;
		subpassDescriptions[1].inputAttachmentCount    = 3;
		subpassDescriptions[1].pInputAttachments       = inputReferences;
		
        std::vector<VkSubpassDependency> dependencies(3);
        dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass      = 0;
        dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        
        dependencies[1].srcSubpass      = 0;
        dependencies[1].dstSubpass      = 1;
        dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[2].srcSubpass      = 1;
        dependencies[2].dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[2].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[2].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[2].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[2].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		
		VkRenderPassCreateInfo renderPassInfo;
		ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
		renderPassInfo.attachmentCount = attachments.size();
		renderPassInfo.pAttachments    = attachments.data();
		renderPassInfo.subpassCount    = subpassDescriptions.size();
		renderPassInfo.pSubpasses      = subpassDescriptions.data();
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies   = dependencies.data();
		VERIFYVULKANRESULT(vkCreateRenderPass(device, &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));
	}

	void DestroyFrameBuffers() override
	{
		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		for (int32 i = 0; i < m_FrameBuffers.size(); ++i) {
			vkDestroyFramebuffer(device, m_FrameBuffers[i], VULKAN_CPU_ALLOCATOR);
		}
		m_FrameBuffers.clear();
	}

	void DestoryRenderPass() override
	{
		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		if (m_RenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(device, m_RenderPass, VULKAN_CPU_ALLOCATOR);
			m_RenderPass = VK_NULL_HANDLE;
		}
	}

private:

	struct ModelBlock
	{
		Matrix4x4 model;
	};

	struct ViewProjectionBlock
	{
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	struct AttachmentParamBlock
	{
		int attachmentIndex;
		float zNear;
		float zFar;
		float padding;
	};
    
	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		UpdateUniform();
		
		DemoBase::Present(bufferIndex);
	}

	void UpdateUniform()
	{
		m_ViewCamera.Perspective(PI / 4.0f, (float)GetWidth(), (float)GetHeight(), m_DebugParam.zNear, m_DebugParam.zFar);
		m_ViewProjData.view = m_ViewCamera.GetView();
		m_ViewProjData.projection = m_ViewCamera.GetProjection();
		m_ViewProjBuffer->CopyFrom(&m_ViewProjData, sizeof(ViewProjectionBlock));

		m_DebugBuffer->CopyFrom(&m_DebugParam, sizeof(AttachmentParamBlock));
	}
    
	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();
        
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("InputAttachmentsDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("Color Depth");
            
			ImGui::Combo("Attachment", &m_DebugParam.attachmentIndex, m_DebugNames.data(), m_DebugNames.size());
			ImGui::SliderFloat("Z-Near", &m_DebugParam.zNear, 0.1f, 3000.0f);
			ImGui::SliderFloat("Z-Far", &m_DebugParam.zFar, 0.1f, 6000.0f);

			if (m_DebugParam.zNear >= m_DebugParam.zFar) {
				m_DebugParam.zNear = m_DebugParam.zFar * 0.5f;
			}

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
		m_Shader0 = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/17_InputAttachments/obj.vert.spv",
			"assets/shaders/17_InputAttachments/obj.frag.spv"
		);

		m_Shader1 = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			"assets/shaders/17_InputAttachments/quad.vert.spv",
			"assets/shaders/17_InputAttachments/quad.frag.spv"
		);

		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// scene model
		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/Room/miniHouse_FBX.FBX",
			m_VulkanDevice,
			cmdBuffer,
			m_Shader0->perVertexAttributes
		);
        
		// quad model
        std::vector<float> vertices = {
            -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f
        };
        std::vector<uint16> indices = {
            0, 1, 2, 0, 2, 3
        };
        
        m_Quad = vk_demo::DVKModel::Create(
            m_VulkanDevice,
            cmdBuffer,
            vertices,
            indices,
            m_Shader1->perVertexAttributes
        );

		delete cmdBuffer;
	}
    
	void DestroyAssets()
	{
		delete m_Model;
        delete m_Quad;

		delete m_Shader0;
		delete m_Shader1;
	}
    
	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[4];
		clearValues[0].color        = { { 0.2f, 0.2f, 0.2f, 0.0f } };
		clearValues[1].color        = { { 0.2f, 0.2f, 0.2f, 0.0f } };
        clearValues[2].color        = { { 0.2f, 0.2f, 0.2f, 0.0f } };
		clearValues[3].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = m_RenderPass;
		renderPassBeginInfo.clearValueCount = 4;
		renderPassBeginInfo.pClearValues    = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
        renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;
        
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

		uint32 alignment  = m_VulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
		uint32 modelAlign = Align(sizeof(ModelBlock), alignment);

		for (int32 i = 0; i < m_CommandBuffers.size(); ++i)
		{
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(m_CommandBuffers[i],  0, 1, &scissor);

			// pass0
			{
				vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipeline);
				for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
					uint32 offset = meshIndex * modelAlign;
					vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipelineLayout, 0, m_DescriptorSet0->descriptorSets.size(), m_DescriptorSet0->descriptorSets.data(), 1, &offset);
					m_Model->meshes[meshIndex]->BindDrawCmd(m_CommandBuffers[i]);
				}
			}

			vkCmdNextSubpass(m_CommandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);

			// pass1
			{
				vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipeline);
				vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipelineLayout, 0, m_DescriptorSets[i]->descriptorSets.size(), m_DescriptorSets[i]->descriptorSets.data(), 0, nullptr);
                for (int32 meshIndex = 0; meshIndex < m_Quad->meshes.size(); ++meshIndex) {
                    m_Quad->meshes[meshIndex]->BindDrawCmd(m_CommandBuffers[i]);
                }
			}
            
			m_GUI->BindDrawCmd(m_CommandBuffers[i], m_RenderPass, 1);

			vkCmdEndRenderPass(m_CommandBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
		}
	}
    
	void CreateDescriptorSet()
	{
		m_DescriptorSet0 = m_Shader0->AllocateDescriptorSet();
		m_DescriptorSet0->WriteBuffer("uboViewProj", m_ViewProjBuffer);
		m_DescriptorSet0->WriteBuffer("uboModel",    m_ModelBuffer);

		m_DescriptorSets.resize(m_AttachsColor.size());
		for (int32 i = 0; i < m_DescriptorSets.size(); ++i)
		{
			m_DescriptorSets[i] = m_Shader1->AllocateDescriptorSet();
			m_DescriptorSets[i]->WriteImage("inputColor", m_AttachsColor[i]);
            m_DescriptorSets[i]->WriteImage("inputNormal", m_AttachsNormal[i]);
			m_DescriptorSets[i]->WriteImage("inputDepth", m_AttachsDepth[i]);
			m_DescriptorSets[i]->WriteBuffer("param", m_DebugBuffer);
		}
	}
    
	void CreatePipelines()
	{
		vk_demo::DVKGfxPipelineInfo pipelineInfo0;
		pipelineInfo0.shader = m_Shader0;
        pipelineInfo0.colorAttachmentCount = 2;
		m_Pipeline0 = vk_demo::DVKGfxPipeline::Create(
			m_VulkanDevice, 
			m_PipelineCache, 
			pipelineInfo0, 
			{ 
				m_Model->GetInputBinding()
			}, 
			m_Model->GetInputAttributes(), 
			m_Shader0->pipelineLayout, 
			m_RenderPass
		);
		
		vk_demo::DVKGfxPipelineInfo pipelineInfo1;
		pipelineInfo1.depthStencilState.depthTestEnable   = VK_FALSE;
		pipelineInfo1.depthStencilState.depthWriteEnable  = VK_FALSE;
		pipelineInfo1.depthStencilState.stencilTestEnable = VK_FALSE;
		pipelineInfo1.shader  = m_Shader1;
		pipelineInfo1.subpass = 1;
		m_Pipeline1 = vk_demo::DVKGfxPipeline::Create(
			m_VulkanDevice, 
			m_PipelineCache, 
			pipelineInfo1, 
			{ 
				m_Quad->GetInputBinding()
			}, 
			m_Quad->GetInputAttributes(), 
			m_Shader1->pipelineLayout, 
			m_RenderPass
		);
	}
    
	void DestroyPipelines()
	{
        delete m_Pipeline0;
		delete m_Pipeline1;

		delete m_DescriptorSet0;
		for (int32 i = 0; i < m_DescriptorSets.size(); ++i)
		{
			vk_demo::DVKDescriptorSet* descriptorSet = m_DescriptorSets[i];
			delete descriptorSet;
		}
		m_DescriptorSets.clear();
	}
	
	void CreateUniformBuffers()
	{
		// dynamic
		uint32 alignment  = m_VulkanDevice->GetLimits().minUniformBufferOffsetAlignment;
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
        
		// debug params
		m_DebugParam.attachmentIndex = 0;
		m_DebugParam.zNear = 300.0f;
		m_DebugParam.zFar = 3000.0f;
		m_DebugBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(AttachmentParamBlock),
			&(m_DebugParam)
		);
		m_DebugBuffer->Map();

		m_DebugNames.push_back("Color");
		m_DebugNames.push_back("Depth");
		m_DebugNames.push_back("Normal");
        
		m_ViewProjBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(ViewProjectionBlock),
			&(m_ViewProjData)
		);
		m_ViewProjBuffer->Map();

		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), m_DebugParam.zNear, m_DebugParam.zFar);
		m_ViewCamera.SetPosition(boundCenter.x, boundCenter.y + 1000, boundCenter.z - boundSize.Size());
		m_ViewCamera.LookAt(boundCenter);
	}
	
	void DestroyUniformBuffers()
	{
		m_ViewProjBuffer->UnMap();
		delete m_ViewProjBuffer;
		m_ViewProjBuffer = nullptr;

		m_ModelBuffer->UnMap();
		delete m_ModelBuffer;
		m_ModelBuffer = nullptr;

		m_DebugBuffer->UnMap();
		delete m_DebugBuffer;
		m_DebugBuffer = nullptr;
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

	typedef std::vector<vk_demo::DVKDescriptorSet*>		DVKDescriptorSetArray;
	typedef std::vector<vk_demo::DVKTexture*>			DVKTextureArray;

	bool 							m_Ready = false;

	vk_demo::DVKCamera				m_ViewCamera;
    
	std::vector<uint8>              m_ModelDatas;
	vk_demo::DVKBuffer*				m_ModelBuffer = nullptr;

	vk_demo::DVKBuffer*				m_ViewProjBuffer = nullptr;
	ViewProjectionBlock				m_ViewProjData;

	vk_demo::DVKBuffer*				m_DebugBuffer = nullptr;
	AttachmentParamBlock			m_DebugParam;
	std::vector<const char*>		m_DebugNames;
    
	vk_demo::DVKModel*				m_Model = nullptr;
    vk_demo::DVKModel*              m_Quad = nullptr;

    vk_demo::DVKGfxPipeline*        m_Pipeline0 = nullptr;
	vk_demo::DVKShader*				m_Shader0 = nullptr;
	vk_demo::DVKDescriptorSet*		m_DescriptorSet0 = nullptr;
	
	vk_demo::DVKGfxPipeline*        m_Pipeline1 = nullptr;
	vk_demo::DVKShader*				m_Shader1 = nullptr;
	DVKDescriptorSetArray			m_DescriptorSets;

	DVKTextureArray					m_AttachsDepth;
	DVKTextureArray					m_AttachsColor;
    DVKTextureArray                 m_AttachsNormal;
	
	ImageGUIContext*				m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<InputAttachmentsDemo>(1400, 900, "InputAttachmentsDemo", cmdLine);
}
