#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class PCFShadowDemo : public DemoBase
{
public:
	PCFShadowDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~PCFShadowDemo()
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

		CreateRenderTarget();
		CreateGUI();
		LoadAssets();
		InitParmas();

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

	struct ModelViewProjectionBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	struct DirectionalLightBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
		Vector4 direction;
	};

	struct ShadowParamBlock
	{
		Vector4 bias;
	};

	void UpdateLight(float time, float delta)
	{
		if (!m_AnimLight) {
			return;
		}
		m_LightCamera.view.SetIdentity();
		m_LightCamera.view.SetOrigin(Vector3(200 * MMath::Sin(time), 700, -500 * MMath::Cos(time)));
		m_LightCamera.view.LookAt(Vector3(0, 0, 0));
		m_LightCamera.direction = -m_LightCamera.view.GetForward().GetSafeNormal();
		m_LightCamera.view.SetInverse();
	}

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();

		UpdateLight(time, delta);

		// depth
		m_DepthMaterial->BeginFrame();
		for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
			m_LightCamera.model = m_ModelScene->meshes[j]->linkNode->GetGlobalMatrix();
			m_DepthMaterial->BeginObject();
			m_DepthMaterial->SetLocalUniform("uboMVP", &m_LightCamera, sizeof(DirectionalLightBlock));
			m_DepthMaterial->EndObject();
		}
		m_DepthMaterial->EndFrame();

		// shade
		vk_demo::DVKMaterial* shadowMaterial = m_ShadowList[m_Selected];
		shadowMaterial->BeginFrame();
		for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
			m_MVPData.model = m_ModelScene->meshes[j]->linkNode->GetGlobalMatrix();
			shadowMaterial->BeginObject();
			shadowMaterial->SetLocalUniform("uboMVP", &m_MVPData, sizeof(ModelViewProjectionBlock));
			shadowMaterial->SetLocalUniform("lightMVP", &m_LightCamera, sizeof(DirectionalLightBlock));
			shadowMaterial->SetLocalUniform("shadowParam", &m_ShadowParam, sizeof(ShadowParamBlock));
			shadowMaterial->EndObject();
		}
		shadowMaterial->EndFrame();

		SetupCommandBuffers(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("PCFShadowDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Checkbox("Auto Spin", &m_AnimLight);

			ImGui::Combo("Shadow", &m_Selected, m_ShadowNames.data(), m_ShadowNames.size());

			ImGui::SliderFloat("Bias", &m_ShadowParam.bias.x, 0.0f, 0.05f, "%.4f");

			if (m_Selected != 0) 
			{
				ImGui::SliderFloat("Step", &m_ShadowParam.bias.y, 0.0f, 10.0f);
			}
			
			ImGui::Text("ShadowMap:%dx%d", m_ShadowMap->width, m_ShadowMap->height);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void CreateRenderTarget()
	{
		m_ShadowMap = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			2048, 2048,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
        
		vk_demo::DVKRenderPassInfo passInfo(m_ShadowMap, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
		m_ShadowRTT = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, passInfo);
	}

	void DestroyRenderTarget()
	{
		delete m_ShadowRTT;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Quad = vk_demo::DVKDefaultRes::fullQuad;

		// room model
		m_ModelScene = vk_demo::DVKModel::LoadFromFile(
			"assets/models/simplify_BOTI_Dreamsong_Bridge1.fbx",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position,
				VertexAttribute::VA_Normal
			}
		);

		// depth
		m_DepthShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/35_PCFShadow/Depth.vert.spv",
			"assets/shaders/35_PCFShadow/Depth.frag.spv"
		);

		m_DepthMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_ShadowRTT,
			m_PipelineCache,
			m_DepthShader
		);
		m_DepthMaterial->pipelineInfo.colorAttachmentCount = 0;
		m_DepthMaterial->PreparePipeline();

		// simple shadow
		m_SimpleShadowShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/35_PCFShadow/SimpleShadow.vert.spv",
			"assets/shaders/35_PCFShadow/SimpleShadow.frag.spv"
		);

		m_SimpleShadowMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_SimpleShadowShader
		);
		m_SimpleShadowMaterial->PreparePipeline();
		m_SimpleShadowMaterial->SetTexture("shadowMap", m_ShadowMap);

		// pcf shadow
		m_PCFShadowShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/35_PCFShadow/PCFShadow.vert.spv",
			"assets/shaders/35_PCFShadow/PCFShadow.frag.spv"
		);

		m_PCFShadowMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_PCFShadowShader
		);
		m_PCFShadowMaterial->PreparePipeline();
		m_PCFShadowMaterial->SetTexture("shadowMap", m_ShadowMap);

		// ui used
		m_ShadowNames.push_back("Simple");
		m_ShadowNames.push_back("PCF");

		m_ShadowList.push_back(m_SimpleShadowMaterial);
		m_ShadowList.push_back(m_PCFShadowMaterial);

		// debug
		m_DebugShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/35_PCFShadow/Debug.vert.spv",
			"assets/shaders/35_PCFShadow/Debug.frag.spv"
		);

		m_DebugMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_DebugShader
		);

		m_DebugMaterial->PreparePipeline();
		m_DebugMaterial->SetTexture("depthTexture", m_ShadowMap);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_ModelScene;

		delete m_DepthShader;
		delete m_DepthMaterial;

		delete m_DebugMaterial;
		delete m_DebugShader;

		delete m_ShadowMap;

		delete m_SimpleShadowShader;
		delete m_SimpleShadowMaterial;

		delete m_PCFShadowShader;
		delete m_PCFShadowMaterial;
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
			m_ShadowRTT->BeginRenderPass(commandBuffer);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DepthMaterial->GetPipeline());
			for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
				m_DepthMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
				m_ModelScene->meshes[j]->BindDrawCmd(commandBuffer);
			}

			m_ShadowRTT->EndRenderPass(commandBuffer);
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

			// shade
			vk_demo::DVKMaterial* shadowMaterial = m_ShadowList[m_Selected];
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMaterial->GetPipeline());
			for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
				shadowMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
				m_ModelScene->meshes[j]->BindDrawCmd(commandBuffer);
			}

			// debug
			viewport.x = m_FrameWidth * 0.75f;
			viewport.y = m_FrameHeight * 0.25f;
			viewport.width  = m_FrameWidth * 0.25f;
			viewport.height = -(float)m_FrameHeight * 0.25f;    // flip y axis

			scissor.offset.x = m_FrameWidth * 0.75f;
			scissor.offset.y = 0;
			scissor.extent.width  = m_FrameWidth  * 0.25f;
			scissor.extent.height = m_FrameHeight * 0.25f;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugMaterial->GetPipeline());
			m_DebugMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_Quad->meshes[0]->BindDrawCmd(commandBuffer);

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.SetPosition(-500, 800, 0);
		m_ViewCamera.LookAt(0, 200, 0);
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 10.0f, 3000.0f);

		m_LightCamera.view.SetIdentity();
		m_LightCamera.view.SetOrigin(Vector3(200, 700, -500));
		m_LightCamera.view.LookAt(Vector3(0, 0, 0));
		m_LightCamera.direction = -m_LightCamera.view.GetForward().GetSafeNormal();
		m_LightCamera.view.SetInverse();

		m_LightCamera.projection.SetIdentity();
		// 正交投影区域能够包裹住场景即可，注意比例要与ShadowMap保持一致。
		m_LightCamera.projection.Orthographic(-600, 600, -600, 600, 100.0f, 1500.0f);

		m_ShadowParam.bias.x = 0.005f;
		m_ShadowParam.bias.y = 5.0f;
		m_ShadowParam.bias.z = 0.0f;
		m_ShadowParam.bias.w = 0.0f;
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

	typedef std::vector<vk_demo::DVKTexture*>			TextureArray;
	typedef std::vector<vk_demo::DVKMaterial*>			MaterialArray;
	typedef std::vector<std::vector<vk_demo::DVKMesh*>> MatMeshArray;

	bool 						m_Ready = false;
	vk_demo::DVKCamera			m_ViewCamera;

	// Debug
	vk_demo::DVKModel*			m_Quad = nullptr;
	vk_demo::DVKMaterial*	    m_DebugMaterial;
	vk_demo::DVKShader*		    m_DebugShader;

	// Shadow Rendertarget
	vk_demo::DVKRenderTarget*   m_ShadowRTT = nullptr;
	vk_demo::DVKTexture*        m_ShadowMap = nullptr;

	// depth 
	vk_demo::DVKShader*			m_DepthShader = nullptr;
	vk_demo::DVKMaterial*		m_DepthMaterial = nullptr;

	// mvp
	ModelViewProjectionBlock	m_MVPData;
	vk_demo::DVKModel*			m_ModelScene = nullptr;

	// light
	DirectionalLightBlock		m_LightCamera;
	ShadowParamBlock			m_ShadowParam;

	// obj render
	vk_demo::DVKShader*			m_SimpleShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_SimpleShadowMaterial = nullptr;

	vk_demo::DVKShader*			m_PCFShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_PCFShadowMaterial = nullptr;

	bool                        m_AnimLight = true;
	int32						m_Selected = 1;
	std::vector<const char*>	m_ShadowNames;
	MaterialArray				m_ShadowList;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<PCFShadowDemo>(1400, 900, "PCFShadowDemo", cmdLine);
}
