#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"
#include "File/FileManager.h"
#include "UI/ImageGUIContext.h"

#include <vector>
#include <fstream>

class InputAttachments : public DemoBase
{
public:
	InputAttachments(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}
    
	virtual ~InputAttachments()
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

		VkImageView attachments[3];

		VkFramebufferCreateInfo frameBufferCreateInfo;
		ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
		frameBufferCreateInfo.renderPass      = m_RenderPass;
		frameBufferCreateInfo.attachmentCount = 3;
		frameBufferCreateInfo.pAttachments    = attachments;
		frameBufferCreateInfo.width			  = fwidth;
		frameBufferCreateInfo.height		  = fheight;
		frameBufferCreateInfo.layers		  = 1;

		const std::vector<VkImageView>& backbufferViews = GetVulkanRHI()->GetBackbufferViews();

		m_FrameBuffers.resize(backbufferViews.size());
		for (uint32 i = 0; i < m_FrameBuffers.size(); ++i) {
			attachments[0] = backbufferViews[i];
			attachments[1] = m_AttachsColor[i]->imageView;
			attachments[2] = m_AttachsDepth[i]->imageView;
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
	}

	void CreateAttachments()
	{
		auto swapChain  = GetVulkanRHI()->GetSwapChain();
		int32 fwidth    = swapChain->GetWidth();
		int32 fheight   = swapChain->GetHeight();
		int32 numBuffer = swapChain->GetBackBufferCount();
		VkDevice device = m_VulkanDevice->GetInstanceHandle();
		
		m_AttachsDepth.resize(numBuffer);
		m_AttachsColor.resize(numBuffer);

		for (int32 i = 0; i < m_AttachsDepth.size(); ++i)
		{
			m_AttachsDepth[i] = vk_demo::DVKTexture::Create2D(
				m_VulkanDevice, 
				PixelFormatToVkFormat(m_DepthFormat, false), 
				VK_IMAGE_ASPECT_DEPTH_BIT,
				fwidth, fheight,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			);
			m_AttachsDepth[i]->descriptorInfo.sampler = VK_NULL_HANDLE;
		}
		
		for (int32 i = 0; i < m_AttachsColor.size(); ++i)
		{
			m_AttachsColor[i] = vk_demo::DVKTexture::Create2D(
				m_VulkanDevice, 
				PixelFormatToVkFormat(GetVulkanRHI()->GetPixelFormat(), false), 
				VK_IMAGE_ASPECT_COLOR_BIT,
				fwidth, fheight,
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
			);
			m_AttachsColor[i]->descriptorInfo.sampler = VK_NULL_HANDLE;
		}
	}

	void CreateRenderPass() override
	{
		DestoryRenderPass();
		DestroyAttachments();
		CreateAttachments();

		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		PixelFormat pixelFormat = GetVulkanRHI()->GetPixelFormat();

		std::vector<VkAttachmentDescription> attachments(3);
		// swap chain attachment
		attachments[0].format		  = PixelFormatToVkFormat(pixelFormat, false);
		attachments[0].samples		  = m_SampleCount;
		attachments[0].loadOp		  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		// color attachment
		attachments[1].format         = PixelFormatToVkFormat(pixelFormat, false);
		attachments[1].samples        = m_SampleCount;
		attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[1].finalLayout	  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// depth stencil attachment
		attachments[2].format         = PixelFormatToVkFormat(m_DepthFormat, false);
		attachments[2].samples        = m_SampleCount;
		attachments[2].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachments[2].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachments[2].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachments[2].finalLayout	  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		VkAttachmentReference swapReference = { };
		swapReference.attachment  = 0;
		swapReference.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference = { };
		colorReference.attachment = 1;
		colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		
		VkAttachmentReference depthReference = { };
		depthReference.attachment = 2;
		depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference inputReferences[2];
		inputReferences[0].attachment = 1;
		inputReferences[0].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		inputReferences[1].attachment = 2;
		inputReferences[1].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::vector<VkSubpassDescription> subpassDescriptions(2);
		subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[0].colorAttachmentCount    = 1;
		subpassDescriptions[0].pColorAttachments       = &colorReference;
		subpassDescriptions[0].pDepthStencilAttachment = &depthReference;
		subpassDescriptions[0].pResolveAttachments     = nullptr;
		subpassDescriptions[0].inputAttachmentCount    = 0;
		subpassDescriptions[0].pInputAttachments       = nullptr;
		subpassDescriptions[0].preserveAttachmentCount = 0;
		subpassDescriptions[0].pPreserveAttachments    = nullptr;

		subpassDescriptions[1].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescriptions[1].colorAttachmentCount    = 1;
		subpassDescriptions[1].pColorAttachments       = &swapReference;
		subpassDescriptions[1].pDepthStencilAttachment = nullptr;
		subpassDescriptions[1].pResolveAttachments     = nullptr;
		subpassDescriptions[1].inputAttachmentCount    = 2;
		subpassDescriptions[1].pInputAttachments       = inputReferences;
		subpassDescriptions[1].preserveAttachmentCount = 0;
		subpassDescriptions[1].pPreserveAttachments    = nullptr;
		
		std::vector<VkSubpassDependency> dependencies(3);
		dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass      = 0;
		dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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
		dependencies[2].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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

	struct ImageInfo
	{
		int32	width  = 0;
		int32	height = 0;
		int32	comp   = 0;
		uint8*	data   = nullptr;
	};
    
	struct MVPBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	void Draw(float time, float delta)
	{
        UpdateUI(time, delta);
        DemoBase::Present();
	}
    
	void UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();
        
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("InputAttachments", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("Color Depth");
            
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
		m_Shader0 = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/16_OptimizeShaderAndLayout/lut.vert.spv",
			"assets/shaders/16_OptimizeShaderAndLayout/lut.frag.spv"
		);

		m_Shader1 = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			"assets/shaders/17_InputAttachments/quad.vert.spv",
			"assets/shaders/17_InputAttachments/quad.frag.spv"
		);

		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// 读取模型文件
		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/plane_z.obj",
			m_VulkanDevice,
			cmdBuffer,
			m_Shader0->attributes
		);
        
		// 生成LUT 3D图数据
		// 64mb 
		// map image0 -> image1
		int32 lutSize  = 256;
		uint8* lutRGBA = new uint8[lutSize * lutSize * 4 * lutSize];
        for (int32 x = 0; x < lutSize; ++x)
        {
            for (int32 y = 0; y < lutSize; ++y)
            {
                for (int32 z = 0; z < lutSize; ++z)
                {
                    int idx = (x + y * lutSize + z * lutSize * lutSize) * 4;
                    int32 r = x * 1.0f / (lutSize - 1) * 255;
                    int32 g = y * 1.0f / (lutSize - 1) * 255;
                    int32 b = z * 1.0f / (lutSize - 1) * 255;
                    // 怀旧PS滤镜，色调映射。
                    r = 0.393f * r + 0.769f * g + 0.189f * b;
                    g = 0.349f * r + 0.686f * g + 0.168f * b;
                    b = 0.272f * r + 0.534f * g + 0.131f * b;
                    lutRGBA[idx + 0] = MMath::Min(r, 255);
                    lutRGBA[idx + 1] = MMath::Min(g, 255);
                    lutRGBA[idx + 2] = MMath::Min(b, 255);
                    lutRGBA[idx + 3] = 255;
                }
            }
        }
        
		// 创建Texture
		m_TexOrigin = vk_demo::DVKTexture::Create2D("assets/textures/game0.jpg", m_VulkanDevice, cmdBuffer);
		m_Tex3DLut  = vk_demo::DVKTexture::Create3D(VK_FORMAT_R8G8B8A8_UNORM, lutRGBA, lutSize * lutSize * 4 * lutSize, lutSize, lutSize, lutSize, m_VulkanDevice, cmdBuffer);
		
		delete cmdBuffer;
	}
    
	void DestroyAssets()
	{
		delete m_Model;

		delete m_TexOrigin;
        delete m_Tex3DLut;

		delete m_Shader0;
		delete m_Shader1;
	}
    
	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[3];
		clearValues[0].color        = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].color        = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[2].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = m_RenderPass;
		renderPassBeginInfo.clearValueCount = 3;
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

		for (int32 i = 0; i < m_CommandBuffers.size(); ++i)
		{
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);

			// pass0
			{
				vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipeline);
				vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipelineLayout, 0, m_DescriptorSet0->descriptorSets.size(), m_DescriptorSet0->descriptorSets.data(), 0, nullptr);
				for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
					m_Model->meshes[meshIndex]->BindDrawCmd(m_CommandBuffers[i]);
				}

				m_GUI->BindDrawCmd(m_CommandBuffers[i], m_RenderPass);
			}

			vkCmdNextSubpass(m_CommandBuffers[i], VK_SUBPASS_CONTENTS_INLINE);

			// pass1
			{
				vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipeline);
				vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipelineLayout, 0, m_DescriptorSets[i]->descriptorSets.size(), m_DescriptorSets[i]->descriptorSets.data(), 0, nullptr);
				vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);
			}

			vkCmdEndRenderPass(m_CommandBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
		}
	}
    
	void CreateDescriptorSet()
	{
		m_DescriptorSet0 = m_Shader0->AllocateDescriptorSet();
		m_DescriptorSet0->WriteBuffer("uboMVP", m_MVPBuffer);
		m_DescriptorSet0->WriteImage("diffuseMap", m_TexOrigin);
		m_DescriptorSet0->WriteImage("lutMap", m_Tex3DLut);

		m_DescriptorSets.resize(m_AttachsColor.size());
		for (int32 i = 0; i < m_DescriptorSets.size(); ++i)
		{
			m_DescriptorSets[i] = m_Shader1->AllocateDescriptorSet();
			m_DescriptorSets[i]->WriteInputAttachment("inputColor", m_AttachsColor[i]);
			m_DescriptorSets[i]->WriteInputAttachment("inputDepth", m_AttachsDepth[i]);
		}
	}
    
	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();
		
		vk_demo::DVKPipelineInfo pipelineInfo0;
		pipelineInfo0.shader = m_Shader0;
		m_Pipeline0 = vk_demo::DVKPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo0, { vertexInputBinding }, vertexInputAttributs, m_Shader0->pipelineLayout, m_RenderPass);
		
		vk_demo::DVKPipelineInfo pipelineInfo1;
		pipelineInfo1.depthStencilState.depthTestEnable   = VK_FALSE;
		pipelineInfo1.depthStencilState.depthWriteEnable  = VK_FALSE;
		pipelineInfo1.depthStencilState.stencilTestEnable = VK_FALSE;
		pipelineInfo1.shader  = m_Shader1;
		pipelineInfo1.subpass = 1;
		m_Pipeline1 = vk_demo::DVKPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo1, { vertexInputBinding }, vertexInputAttributs, m_Shader1->pipelineLayout, m_RenderPass);
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
		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
        Vector3 boundCenter = bounds.min + boundSize * 0.5f;
		boundCenter.z = -10.0f;

		// mvp数据
		m_MVPData.model.SetIdentity();
		m_MVPData.model.SetOrigin(Vector3(0, 0, 0));
		m_MVPData.model.AppendScale(Vector3(1.0f, 0.5f, 1.0f));
        
		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(boundCenter);
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(MMath::DegreesToRadians(75.0f), (float)GetWidth() / 2, (float)GetHeight() / 2, 0.01f, 3000.0f);
		
		m_MVPBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(MVPBlock),
			&(m_MVPData)
		);
		m_MVPBuffer->Map();
	}
	
	void DestroyUniformBuffers()
	{
		m_MVPBuffer->UnMap();
		delete m_MVPBuffer;
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

	typedef std::vector<vk_demo::DVKDescriptorSet*>		DVKDescriptorSetArray;
	typedef std::vector<vk_demo::DVKTexture*>			DVKTextureArray;

	bool 							m_Ready = false;
    
	MVPBlock 						m_MVPData;
	vk_demo::DVKBuffer*				m_MVPBuffer;
    
	vk_demo::DVKTexture*			m_TexOrigin = nullptr;
	vk_demo::DVKTexture*			m_Tex3DLut = nullptr;

	vk_demo::DVKModel*				m_Model = nullptr;

    vk_demo::DVKPipeline*           m_Pipeline0 = nullptr;
	vk_demo::DVKShader*				m_Shader0 = nullptr;
	vk_demo::DVKDescriptorSet*		m_DescriptorSet0 = nullptr;
	
	vk_demo::DVKPipeline*           m_Pipeline1 = nullptr;
	vk_demo::DVKShader*				m_Shader1 = nullptr;
	DVKDescriptorSetArray			m_DescriptorSets;

	DVKTextureArray					m_AttachsDepth;
	DVKTextureArray					m_AttachsColor;
	
	ImageGUIContext*				m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<InputAttachments>(1400, 900, "InputAttachments", cmdLine);
}