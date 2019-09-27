#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

#define TEX_SIZE 4

class SSAODemo : public DemoBase
{
public:
	SSAODemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~SSAODemo()
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
			ImGui::Begin("SSAODemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void CreateSourceRT()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_TexSourceColor = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			cmdBuffer,
			m_SwapChain->GetColorFormat(),
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_TexSourceDepth0 = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			cmdBuffer,
			VK_FORMAT_R32_SFLOAT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
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
		rtColors[1] = m_TexSourceDepth0;

		vk_demo::DVKRenderPassInfo rttInfo(
			2, rtColors, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			m_TexSourceDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
		);
		m_RTSource = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttInfo);
		m_RTSource->colorLayout = ImageLayoutBarrier::ComputeGeneralRW;

		delete cmdBuffer;
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
				VertexAttribute::VA_UV0,
				VertexAttribute::VA_Normal
			}
		);

		m_SceneShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/53_SSAO/obj.vert.spv",
			"assets/shaders/53_SSAO/obj.frag.spv"
		);

		const char* textures[TEX_SIZE] = {
			"assets/models/Room/miniHouse_Part1.jpg",
			"assets/models/Room/miniHouse_Part2.jpg",
			"assets/models/Room/miniHouse_Part3.jpg",
			"assets/models/Room/miniHouse_Part4.jpg"
		};

		for (int32 i = 0; i < TEX_SIZE; ++i)
		{
			m_SceneTextures[i] = vk_demo::DVKTexture::Create2D(
				textures[i],
				m_VulkanDevice,
				cmdBuffer
			);

			m_SceneMaterials[i] = vk_demo::DVKMaterial::Create(
				m_VulkanDevice,
				m_RTSource->GetRenderPass(),
				m_PipelineCache,
				m_SceneShader
			);
			m_SceneMaterials[i]->pipelineInfo.colorAttachmentCount = 2;
			m_SceneMaterials[i]->PreparePipeline();
			m_SceneMaterials[i]->SetTexture("diffuseMap", m_SceneTextures[i]);
		}

		// compute
		m_ComputeShader = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/42_OptimizeComputeShader/ColorInvert.comp.spv"
		);

		m_ComputeProcessor = vk_demo::DVKCompute::Create(
			m_VulkanDevice, 
			m_PipelineCache, 
			m_ComputeShader
		);
		m_ComputeProcessor->SetStorageTexture("inputImage",  m_TexSourceColor);
		m_ComputeProcessor->SetStorageTexture("outputImage", m_TexSourceColor);

		m_ComputeCommand = vk_demo::DVKCommandBuffer::Create(
			m_VulkanDevice, 
			m_ComputeCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			m_VulkanDevice->GetComputeQueue()
		);

		// final
		m_FinalShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/53_SSAO/combine.vert.spv",
			"assets/shaders/53_SSAO/combine.frag.spv"
		);

		m_FinalMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FinalShader
		);
		m_FinalMaterial->PreparePipeline();
		m_FinalMaterial->SetTexture("originTexture",    m_TexSourceColor);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_SceneModel;
		delete m_SceneShader;

		for (int32 i = 0; i < TEX_SIZE; ++i)
		{
			delete m_SceneTextures[i];
			delete m_SceneMaterials[i];
		}

		// source
		{
			delete m_TexSourceColor;
			delete m_TexSourceDepth0;
			delete m_TexSourceDepth;
			delete m_RTSource;
		}

		// compute
		{
			delete m_ComputeShader;
			delete m_ComputeProcessor;
			delete m_ComputeCommand;
		}

		// final
		{
			delete m_FinalShader;
			delete m_FinalMaterial;
		}
	}

	void SourcePass(VkCommandBuffer commandBuffer)
	{
		m_RTSource->BeginRenderPass(commandBuffer);

		vk_demo::DVKMaterial* materials[7] = {
			m_SceneMaterials[3],
			m_SceneMaterials[2],
			m_SceneMaterials[1],
			m_SceneMaterials[0],
			m_SceneMaterials[0],
			m_SceneMaterials[3],
			m_SceneMaterials[0],
		};

		for (int32 i = 0; i < m_SceneModel->meshes.size(); ++i)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materials[i]->GetPipeline());

			materials[i]->BeginFrame();

			m_MVPParam.model = m_SceneModel->meshes[i]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = m_ViewCamera.GetView();
			m_MVPParam.proj  = m_ViewCamera.GetProjection();

			materials[i]->BeginObject();
			materials[i]->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
			materials[i]->EndObject();

			materials[i]->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_SceneModel->meshes[i]->BindDrawCmd(commandBuffer);

			materials[i]->EndFrame();
		}

		m_RTSource->EndRenderPass(commandBuffer);
	}

	void RenderFinal(VkCommandBuffer commandBuffer, int32 backBufferIndex)
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

		// combine pass
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

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			m_FinalMaterial->BeginFrame();

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_FinalMaterial->GetPipeline());
			m_FinalMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_Quad->meshes[0]->BindDrawCmd(commandBuffer);

			m_FinalMaterial->EndFrame();
		}

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
		RenderFinal(commandBuffer, backBufferIndex);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.SetPosition(0, 100.0f, -1000.0f);
		m_ViewCamera.LookAt(0, 100.0f, 0);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 10.0f, 3000.0f);
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
	vk_demo::DVKTexture*		m_SceneTextures[TEX_SIZE];
	vk_demo::DVKMaterial*		m_SceneMaterials[TEX_SIZE];

	// source
	vk_demo::DVKTexture*		m_TexSourceColor = nullptr;
	vk_demo::DVKTexture*		m_TexSourceDepth0 = nullptr;
	vk_demo::DVKTexture*		m_TexSourceDepth = nullptr;
	vk_demo::DVKRenderTarget*	m_RTSource = nullptr;

	// compute
	vk_demo::DVKShader*         m_ComputeShader = nullptr;
	vk_demo::DVKCompute*   		m_ComputeProcessor = nullptr;
	vk_demo::DVKCommandBuffer*	m_ComputeCommand = nullptr;

	// finnal pass
	vk_demo::DVKShader*			m_FinalShader = nullptr;
	vk_demo::DVKMaterial*		m_FinalMaterial = nullptr;

	vk_demo::DVKCamera		    m_ViewCamera;

	ModelViewProjectionBlock	m_MVPParam;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<SSAODemo>(1400, 900, "SSAODemo", cmdLine);
}
