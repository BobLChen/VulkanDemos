#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"
#include "Demo/DVKTexture.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"
#include "File/FileManager.h"
#include "UI/ImageGUIContext.h"

#include <vector>
#include <fstream>

class RenderTargetDemo : public DemoBase
{
public:
	RenderTargetDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~RenderTargetDemo()
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

		CreateGUI();
		LoadAssets();
		InitParmas();
		CreateRenderTarget();

		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
		DemoBase::Release();

		DestroyRenderTarget();
		DestroyAssets();
		DestroyGUI();
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

	struct FrameBufferObject
	{
		int32					width = 0;
		int32					height = 0;

		VkDevice				device = VK_NULL_HANDLE;
		VkFramebuffer			frameBuffer = VK_NULL_HANDLE;
		VkRenderPass			renderPass = VK_NULL_HANDLE;

		vk_demo::DVKTexture*	color = nullptr;
		vk_demo::DVKTexture*	depth = nullptr;

		void Destroy()
		{
			if (color) {
				delete color;
				color = nullptr;
			}

			if (depth) {
				delete depth;
				depth = nullptr;
			}

			if (frameBuffer != VK_NULL_HANDLE) {
				vkDestroyFramebuffer(device, frameBuffer, VULKAN_CPU_ALLOCATOR);
				frameBuffer = VK_NULL_HANDLE;
			}

			if (renderPass != VK_NULL_HANDLE) {
				vkDestroyRenderPass(device, renderPass, VULKAN_CPU_ALLOCATOR);
				renderPass = VK_NULL_HANDLE;
			}
		}
	};

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateUI(time, delta);

		m_Material->BeginFrame();
		for (int32 i = 0; i < m_Model->meshes.size(); ++i) {
			m_Material->BeginObject();
			m_Material->SetLocalUniform("uboMVP", &m_MVPData, sizeof(MVPBlock));
			m_Material->EndObject();
		}
		m_Material->EndFrame();

		SetupCommandBuffers(bufferIndex);
		DemoBase::Present(bufferIndex);
	}

	void UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("RenderTargetDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		m_GUI->EndFrame();
		m_GUI->Update();
	}

	void CreateRenderTarget()
	{
		m_RenderTarget.device = m_Device;
		m_RenderTarget.width  = m_FrameWidth;
		m_RenderTarget.height = m_FrameHeight;

		m_RenderTarget.color = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice, 
			PixelFormatToVkFormat(GetVulkanRHI()->GetPixelFormat(), false), 
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		m_RenderTarget.depth = vk_demo::DVKTexture::Create2D(
            m_VulkanDevice,
            PixelFormatToVkFormat(m_DepthFormat, false),
            VK_IMAGE_ASPECT_DEPTH_BIT,
            m_FrameWidth, m_FrameHeight,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );

		std::vector<VkAttachmentDescription> attchmentDescriptions(2);
		// Color attachment
		attchmentDescriptions[0].format         = m_RenderTarget.color->format;
		attchmentDescriptions[0].samples        = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[0].finalLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		// Depth attachment
		attchmentDescriptions[1].format         = m_RenderTarget.depth->format;
		attchmentDescriptions[1].samples        = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorReference;
		colorReference.attachment = 0;
		colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthReference;
		depthReference.attachment = 1;
		depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount    = 1;
		subpassDescription.pColorAttachments       = &colorReference;
		subpassDescription.pDepthStencilAttachment = &depthReference;

		std::vector<VkSubpassDependency> dependencies(2);
		dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass      = 0;
		dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask   = VK_ACCESS_SHADER_READ_BIT;
		dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass      = 0;
		dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create renderpass
		VkRenderPassCreateInfo renderPassInfo;
		ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
		renderPassInfo.attachmentCount = attchmentDescriptions.size();
		renderPassInfo.pAttachments    = attchmentDescriptions.data();
		renderPassInfo.subpassCount    = 1;
		renderPassInfo.pSubpasses      = &subpassDescription;
		renderPassInfo.dependencyCount = dependencies.size();
		renderPassInfo.pDependencies   = dependencies.data();
		VERIFYVULKANRESULT(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &(m_RenderTarget.renderPass)));
		
		VkImageView attachments[2];
		attachments[0] = m_RenderTarget.color->imageView;
		attachments[1] = m_RenderTarget.depth->imageView;

		VkFramebufferCreateInfo frameBufferInfo;
		ZeroVulkanStruct(frameBufferInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
		frameBufferInfo.renderPass      = m_RenderTarget.renderPass;
		frameBufferInfo.attachmentCount = 2;
		frameBufferInfo.pAttachments    = attachments;
		frameBufferInfo.width           = m_RenderTarget.width;
		frameBufferInfo.height          = m_RenderTarget.height;
		frameBufferInfo.layers          = 1;
		VERIFYVULKANRESULT(vkCreateFramebuffer(m_Device, &frameBufferInfo, VULKAN_CPU_ALLOCATOR, &(m_RenderTarget.frameBuffer)));
	}

	void DestroyRenderTarget()
	{
		m_RenderTarget.Destroy();
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_TexOrigin = vk_demo::DVKTexture::Create2D("assets/textures/game0.jpg", m_VulkanDevice, cmdBuffer);

		m_Shader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/22_RenderTarget/texture.vert.spv",
			"assets/shaders/22_RenderTarget/texture.frag.spv"
		);

		m_Material = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader
		);
		m_Material->PreparePipeline();
		m_Material->SetTexture("diffuseMap", m_TexOrigin);

		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/plane_z.obj",
			m_VulkanDevice,
			cmdBuffer,
			m_Shader->attributes
		);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;
		delete m_Material;
		delete m_Shader;
		delete m_TexOrigin;
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
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
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		// render target pass
		{
			VkClearValue clearValues[2];
			clearValues[0].color        = { { 0.0f, 0.0f, 0.0f, 0.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo;
			ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
			renderPassBeginInfo.renderPass               = m_RenderTarget.renderPass;
			renderPassBeginInfo.framebuffer              = m_RenderTarget.frameBuffer;
			renderPassBeginInfo.renderArea.offset.x      = 0;
			renderPassBeginInfo.renderArea.offset.y      = 0;
			renderPassBeginInfo.renderArea.extent.width  = m_RenderTarget.width;
			renderPassBeginInfo.renderArea.extent.height = m_RenderTarget.height;
			renderPassBeginInfo.clearValueCount          = 2;
			renderPassBeginInfo.pClearValues             = clearValues;
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());
			for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
				m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Model->meshes[meshIndex]->BindDrawCmd(commandBuffer);
			}

			vkCmdEndRenderPass(commandBuffer);
		}

		// second pass
		{
			VkClearValue clearValues[2];
			clearValues[0].color        = { { 0.2f, 0.2f, 0.2f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo;
			ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
			renderPassBeginInfo.renderPass               = m_RenderPass;
			renderPassBeginInfo.framebuffer              = m_FrameBuffers[backBufferIndex];
			renderPassBeginInfo.clearValueCount          = 2;
			renderPassBeginInfo.pClearValues             = clearValues;
			renderPassBeginInfo.renderArea.offset.x      = 0;
			renderPassBeginInfo.renderArea.offset.y      = 0;
			renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
			renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());
			for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
				m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Model->meshes[meshIndex]->BindDrawCmd(commandBuffer);
			}

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;
		boundCenter.z       = -10.0f;

		m_MVPData.model.SetIdentity();
		m_MVPData.model.SetOrigin(Vector3(0, 0, 0));
		m_MVPData.model.AppendScale(Vector3(1.0f, 0.5f, 1.0f));

		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(boundCenter);
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(MMath::DegreesToRadians(75.0f), (float)GetWidth(), (float)GetHeight(), 0.1f, 3000.0f);
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

	bool 							m_Ready = false;

	MVPBlock 						m_MVPData;
	FrameBufferObject				m_RenderTarget;

	vk_demo::DVKTexture*			m_TexOrigin = nullptr;
	vk_demo::DVKShader*				m_Shader = nullptr;
	vk_demo::DVKMaterial*			m_Material = nullptr;
	vk_demo::DVKModel*				m_Model = nullptr;

	ImageGUIContext*				m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<RenderTargetDemo>(1400, 900, "RenderTargetDemo", cmdLine);
}
