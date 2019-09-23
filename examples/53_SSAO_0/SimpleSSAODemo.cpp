#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class SimpleSSAODemo : public DemoBase
{
public:
	SimpleSSAODemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~SimpleSSAODemo()
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
		InitParmas();
		CreateSourceRT();
		//CreateBlurRT();
		LoadAssets();

		m_Ready = true;
		return true;
	}

	virtual void Exist() override
	{
		DestroyAssets();
		DestroyGUI();
		DemoBase::Release();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

private:

	struct ModelViewProjectionBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 proj;
	};

	struct ParamBlock
	{
		Vector4		data;
		Matrix4x4   view;
		Matrix4x4	invView;
		Matrix4x4	proj;
	};

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		SetupCommandBuffers(bufferIndex);
		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("SimpleSSAODemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void CreateBlurRT()
	{
		// blurH
		m_TexBlurH = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			m_TexAoOcclusion->format,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_TexAoOcclusion->width * 0.25f, m_TexAoOcclusion->height * 0.25f,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKRenderPassInfo rttInfoH(
			m_TexBlurH, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, nullptr
		);
		m_RTBlurH = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttInfoH);

		m_BlurHShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/53_SSAO_0/blurH.vert.spv",
			"assets/shaders/53_SSAO_0/blurH.frag.spv"
		);

		m_BlurHMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RTBlurH->GetRenderPass(),
			m_PipelineCache,
			m_BlurHShader
		);
		m_BlurHMaterial->PreparePipeline();
		m_BlurHMaterial->SetTexture("originTexture", m_TexAoOcclusion);

		// blurV
		m_TexBlurV = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			m_TexBlurH->format, 
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_TexBlurH->width, m_TexBlurH->height,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKRenderPassInfo rttInfoV(
			m_TexBlurV, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, nullptr
		);
		m_RTBlurV = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttInfoV);

		m_BlurVShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/53_SSAO_0/blurV.vert.spv",
			"assets/shaders/53_SSAO_0/blurV.frag.spv"
		);

		m_BlurVMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RTBlurV->GetRenderPass(),
			m_PipelineCache,
			m_BlurVShader
		);
		m_BlurVMaterial->PreparePipeline();
		m_BlurVMaterial->SetTexture("originTexture", m_TexBlurH);
	}

	void CreateSourceRT()
	{
		m_TexSourceColor = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(GetVulkanRHI()->GetPixelFormat(), false),
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		m_TexSourceNormal = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(GetVulkanRHI()->GetPixelFormat(), false),
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		m_TexSourceDepth = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKTexture* rtColors[2];
		rtColors[0] = m_TexSourceColor;
		rtColors[1] = m_TexSourceNormal;

		vk_demo::DVKRenderPassInfo rttInfo(
			2, rtColors, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			m_TexSourceDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
		);
		m_RTSource = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttInfo);
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// fullscreen
		m_Quad = vk_demo::DVKDefaultRes::fullQuad;

		// scene model
		m_SceneModel = vk_demo::DVKModel::LoadFromFile(
			"assets/models/Room/miniHouse_FBX.FBX",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position,
				VertexAttribute::VA_Normal
			}
		);

		m_SceneShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/53_SSAO_0/obj.vert.spv",
			"assets/shaders/53_SSAO_0/obj.frag.spv"
		);

		m_SceneMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RTSource->GetRenderPass(),
			m_PipelineCache,
			m_SceneShader
		);
		m_SceneMaterial->pipelineInfo.colorAttachmentCount = 2;
		m_SceneMaterial->PreparePipeline();

		// final
		m_FinalShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/53_SSAO_0/combine.vert.spv",
			"assets/shaders/53_SSAO_0/combine.frag.spv"
		);

		m_FinalMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FinalShader
		);
		m_FinalMaterial->PreparePipeline();
		m_FinalMaterial->SetTexture("colorTexture",		m_TexSourceColor);
		m_FinalMaterial->SetTexture("normalTexture",	m_TexSourceNormal);
		m_FinalMaterial->SetTexture("depthTexture",		m_TexSourceDepth);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		// scene
		delete m_SceneModel;
		delete m_SceneShader;
		delete m_SceneMaterial;

		// source
		delete m_TexSourceColor;
		delete m_TexSourceNormal;
		delete m_TexSourceDepth;
		delete m_RTSource;

		// blur h
		delete m_TexBlurH;
		delete m_RTBlurH;
		delete m_BlurHShader;
		delete m_BlurHMaterial;

		// blur v
		delete m_TexBlurV;
		delete m_RTBlurV;
		delete m_BlurVShader;
		delete m_BlurVMaterial;

		// final
		delete m_FinalShader;
		delete m_FinalMaterial;
	}

	void SourcePass(VkCommandBuffer commandBuffer)
	{
		m_RTSource->BeginRenderPass(commandBuffer);
		
		for (int32 i = 0; i < m_SceneModel->meshes.size(); ++i)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SceneMaterial->GetPipeline());

			m_SceneMaterial->BeginFrame();

			m_MVPParam.model = m_SceneModel->meshes[i]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = m_ViewCamera.GetView();
			m_MVPParam.proj  = m_ViewCamera.GetProjection();

			m_SceneMaterial->BeginObject();
			m_SceneMaterial->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
			m_SceneMaterial->EndObject();

			m_SceneMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_SceneModel->meshes[i]->BindDrawCmd(commandBuffer);

			m_SceneMaterial->EndFrame();
		}

		m_RTSource->EndRenderPass(commandBuffer);
	}

	void BlurHPass(VkCommandBuffer commandBuffer)
	{
		m_RTBlurH->BeginRenderPass(commandBuffer);
		m_BlurHMaterial->BeginFrame();

		m_BlurHMaterial->BeginObject();



		m_BlurHMaterial->EndObject();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_BlurHMaterial->GetPipeline());
		m_BlurHMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		m_Quad->meshes[0]->BindDrawCmd(commandBuffer);

		m_BlurHMaterial->EndFrame();
		m_RTBlurH->EndRenderPass(commandBuffer);
	}

	void BlurVPass(VkCommandBuffer commandBuffer)
	{
		m_RTBlurV->BeginRenderPass(commandBuffer);
		m_BlurVMaterial->BeginFrame();

		m_BlurVMaterial->BeginObject();



		m_BlurVMaterial->EndObject();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_BlurVMaterial->GetPipeline());
		m_BlurVMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		m_Quad->meshes[0]->BindDrawCmd(commandBuffer);

		m_BlurVMaterial->EndFrame();
		m_RTBlurV->EndRenderPass(commandBuffer);
	}

	void RenderFinal(VkCommandBuffer commandBuffer, int32 backBufferIndex)
	{
		VkViewport viewport = {};
		viewport.x        = 0;
		viewport.y        = m_FrameHeight;
		viewport.width    = m_FrameWidth;
		viewport.height   = -m_FrameHeight;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = m_FrameWidth;
		scissor.extent.height = m_FrameHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

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

		// combine pass
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

		m_FinalMaterial->BeginFrame();

		m_FinalMaterial->BeginObject();
		m_FinalMaterial->SetLocalUniform("param", &m_ParamData, sizeof(ParamBlock));
		m_FinalMaterial->EndObject();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_FinalMaterial->GetPipeline());
		m_FinalMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		m_Quad->meshes[0]->BindDrawCmd(commandBuffer);

		m_FinalMaterial->EndFrame();

		// ui pass
		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		SourcePass(commandBuffer);
		//BlurHPass(commandBuffer);
		//BlurVPass(commandBuffer);
		RenderFinal(commandBuffer, backBufferIndex);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.SetRotation(Vector3(25.131f, 0.619f, 0));
		m_ViewCamera.SetPosition(-26.10247f, 602.182, -890.2283f);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 100.0f, 3000.0f);

		m_ParamData.data.x = m_ViewCamera.GetNear();
		m_ParamData.data.y = m_ViewCamera.GetFar();
		m_ParamData.data.z = m_ViewCamera.GetFar() * MMath::Tan(PI / 4 / 2);
		m_ParamData.data.w = m_ParamData.data.z * (float)GetWidth() / (float)GetHeight();

		m_ParamData.view = m_ViewCamera.GetView();

		m_ParamData.invView = m_ViewCamera.GetView();
		m_ParamData.invView.SetInverse();

		m_ParamData.proj = m_ViewCamera.GetProjection();
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

	bool 						m_Ready = false;

	vk_demo::DVKModel*			m_Quad = nullptr;

	// scene
	vk_demo::DVKModel*			m_SceneModel = nullptr;
	vk_demo::DVKShader*			m_SceneShader = nullptr;
	vk_demo::DVKMaterial*		m_SceneMaterial = nullptr;

	// source
	vk_demo::DVKTexture*		m_TexSourceColor = nullptr;
	vk_demo::DVKTexture*		m_TexSourceNormal = nullptr;
	vk_demo::DVKTexture*		m_TexSourceDepth = nullptr;
	vk_demo::DVKRenderTarget*	m_RTSource = nullptr;

	// Ao pass
	vk_demo::DVKTexture*		m_TexAoOcclusion = nullptr;
	vk_demo::DVKRenderTarget*	m_RTAoOcclusion = nullptr;
	vk_demo::DVKShader*			m_AoOcclusionShader = nullptr;
	vk_demo::DVKMaterial*		m_AoOcclusionMaterial = nullptr;

	// blur h pass
	vk_demo::DVKTexture*		m_TexBlurH = nullptr;
	vk_demo::DVKRenderTarget*	m_RTBlurH = nullptr;
	vk_demo::DVKShader*			m_BlurHShader = nullptr;
	vk_demo::DVKMaterial*		m_BlurHMaterial = nullptr;

	// blur v pass
	vk_demo::DVKTexture*		m_TexBlurV = nullptr;
	vk_demo::DVKRenderTarget*	m_RTBlurV = nullptr;
	vk_demo::DVKShader*			m_BlurVShader = nullptr;
	vk_demo::DVKMaterial*		m_BlurVMaterial = nullptr;

	// finnal pass
	vk_demo::DVKShader*			m_FinalShader = nullptr;
	vk_demo::DVKMaterial*		m_FinalMaterial = nullptr;

	vk_demo::DVKCamera		    m_ViewCamera;

	ModelViewProjectionBlock	m_MVPParam;
	ParamBlock					m_ParamData;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<SimpleSSAODemo>(1400, 900, "SimpleSSAODemo", cmdLine);
}
