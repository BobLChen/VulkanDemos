#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class OmniShadowDemo : public DemoBase
{
public:
	OmniShadowDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
		deviceExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
		instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
	}

	virtual ~OmniShadowDemo()
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

	struct LightCameraParamBlock
	{
		Matrix4x4 model;
		Matrix4x4 view[6];
		Matrix4x4 projection;
		Vector4 position;
	};

	struct ShadowParamBlock
	{
		Vector4 position;
		Vector4 bias;
	};

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

		m_LightCamera.position = m_LightPosition;
		m_ShadowParam.position = m_LightPosition;

		// depth
		// POSITIVE_X
		m_LightCamera.view[0].SetIdentity();
		m_LightCamera.view[0].SetOrigin(Vector3(0, m_LightPosition.y, 0));
		m_LightCamera.view[0].LookAt(Vector3(1, m_LightPosition.y, 0));
		m_LightCamera.view[0].SetInverse();
		// NEGATIVE_X
		m_LightCamera.view[1].SetIdentity();
		m_LightCamera.view[1].SetOrigin(Vector3(0, m_LightPosition.y, 0));
		m_LightCamera.view[1].LookAt(Vector3(-1, m_LightPosition.y, 0));
		m_LightCamera.view[1].SetInverse();
		// POSITIVE_Y
		m_LightCamera.view[2].SetIdentity();
		m_LightCamera.view[2].SetOrigin(Vector3(0, m_LightPosition.y, 0));
		m_LightCamera.view[2].LookAt(Vector3(0, m_LightPosition.y + 1, 0));
		m_LightCamera.view[2].SetInverse();
		// NEGATIVE_Y
		m_LightCamera.view[3].SetIdentity();
		m_LightCamera.view[3].SetOrigin(Vector3(0, m_LightPosition.y, 0));
		m_LightCamera.view[3].LookAt(Vector3(0, m_LightPosition.y - 1, 0));
		m_LightCamera.view[3].SetInverse();
		// POSITIVE_Z
		m_LightCamera.view[4].SetIdentity();
		m_LightCamera.view[4].SetOrigin(Vector3(0, m_LightPosition.y, 0));
		m_LightCamera.view[4].LookAt(Vector3(0, m_LightPosition.y, 1));
		m_LightCamera.view[4].SetInverse();
		// NEGATIVE_Z
		m_LightCamera.view[5].SetIdentity();
		m_LightCamera.view[5].SetOrigin(Vector3(0, m_LightPosition.y, 0));
		m_LightCamera.view[5].LookAt(Vector3(0, m_LightPosition.y, -1));
		m_LightCamera.view[5].SetInverse();
		
		m_DepthMaterial->BeginFrame();
		for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) {
			m_LightCamera.model = m_ModelScene->meshes[j]->linkNode->GetGlobalMatrix();
			m_DepthMaterial->BeginObject();
			m_DepthMaterial->SetLocalUniform("uboMVP", &m_LightCamera, sizeof(LightCameraParamBlock));
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
			shadowMaterial->SetLocalUniform("lightParam", &m_ShadowParam, sizeof(ShadowParamBlock));
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
			ImGui::Begin("OmniShadowDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Combo("Shadow", &m_Selected, m_ShadowNames.data(), m_ShadowNames.size());

			ImGui::SliderFloat("Bias", &m_ShadowParam.bias.x, 0.0f, 20.0f, "%.4f");
			if (m_Selected != 0) {
				ImGui::SliderFloat("Step", &m_ShadowParam.bias.y, 0.0f, 1.0f);
			}

			ImGui::SliderFloat("Light Range", &m_LightPosition.w, 100.0f, 500.0f);
			
			ImGui::Text("ShadowMap:%dx%d", m_RTColor->width, m_RTColor->height);
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
		m_RTColor = vk_demo::DVKTexture::CreateCubeRenderTarget(
			m_VulkanDevice,
			VK_FORMAT_R32_SFLOAT, 
			VK_IMAGE_ASPECT_COLOR_BIT,
			512, 512,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
        
		m_RTDepth = vk_demo::DVKTexture::CreateCubeRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			512, 512,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
        
		vk_demo::DVKRenderPassInfo passInfo(
			m_RTColor, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			m_RTDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
		);
		passInfo.multiview = true;
		m_ShadowRTT = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, passInfo);
	}

	void DestroyRenderTarget()
	{
		delete m_ShadowRTT;
		delete m_RTColor;
		delete m_RTDepth;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

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
			"assets/shaders/36_OmniShadow/Depth.vert.spv",
			"assets/shaders/36_OmniShadow/Depth.frag.spv"
		);

		m_DepthMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_ShadowRTT,
			m_PipelineCache,
			m_DepthShader
		);
		m_DepthMaterial->PreparePipeline();

		// simple shadow
		m_SimpleShadowShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/36_OmniShadow/SimpleShadow.vert.spv",
			"assets/shaders/36_OmniShadow/SimpleShadow.frag.spv"
		);

		m_SimpleShadowMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_SimpleShadowShader
		);
		m_SimpleShadowMaterial->PreparePipeline();
		m_SimpleShadowMaterial->SetTexture("shadowMap", m_RTColor);

		// pcf shadow
		m_PCFShadowShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/36_OmniShadow/PCFShadow.vert.spv",
			"assets/shaders/36_OmniShadow/PCFShadow.frag.spv"
		);

		m_PCFShadowMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_PCFShadowShader
		);
		m_PCFShadowMaterial->PreparePipeline();
		m_PCFShadowMaterial->SetTexture("shadowMap", m_RTColor);

		// ui used
		m_ShadowNames.push_back("Simple");
		m_ShadowNames.push_back("PCF");

		m_ShadowList.push_back(m_SimpleShadowMaterial);
		m_ShadowList.push_back(m_PCFShadowMaterial);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_ModelScene;

		delete m_DepthShader;
		delete m_DepthMaterial;

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
			for (int32 j = 0; j < m_ModelScene->meshes.size(); ++j) 
			{
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

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		vk_demo::DVKBoundingBox bounds = m_ModelScene->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		m_LightPosition.Set(boundCenter.x, boundCenter.y + 50.0f, boundCenter.z, 325.0f);

		m_LightCamera.model.SetIdentity();
		m_LightCamera.projection.SetIdentity();
		m_LightCamera.projection.Perspective(PI / 2.0f, 1.0f, 1.0f, 1.0f, 1500.0f);

		m_ShadowParam.bias.Set(1.5f, 0.005f, 0.0f, 0.0f);

		m_ViewCamera.SetPosition(-500, 800, 0);
		m_ViewCamera.LookAt(0, 200, 0);
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 10.0f, 3000.0f);
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

	// Shadow Rendertarget
	vk_demo::DVKRenderTarget*   m_ShadowRTT = nullptr;
	vk_demo::DVKTexture*        m_RTDepth = nullptr;
	vk_demo::DVKTexture*		m_RTColor = nullptr;

	// depth 
	vk_demo::DVKShader*			m_DepthShader = nullptr;
	vk_demo::DVKMaterial*		m_DepthMaterial = nullptr;

	// mvp
	ModelViewProjectionBlock	m_MVPData;
	vk_demo::DVKModel*			m_ModelScene = nullptr;

	// light
	LightCameraParamBlock		m_LightCamera;
	ShadowParamBlock			m_ShadowParam;

	// obj render
	vk_demo::DVKShader*			m_SimpleShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_SimpleShadowMaterial = nullptr;

	vk_demo::DVKShader*			m_PCFShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_PCFShadowMaterial = nullptr;

	Vector4						m_LightPosition;

	int32						m_Selected = 1;
	std::vector<const char*>	m_ShadowNames;
	MaterialArray				m_ShadowList;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<OmniShadowDemo>(1400, 900, "OmniShadowDemo", cmdLine);
}
