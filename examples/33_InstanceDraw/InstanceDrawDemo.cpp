#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"
#include "Demo/DVKTexture.h"
#include "Demo/DVKRenderTarget.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"
#include "File/FileManager.h"
#include "UI/ImageGUIContext.h"

#include <vector>
#include <fstream>

class InstanceDrawDemo : public DemoBase
{
public:
	InstanceDrawDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~InstanceDrawDemo()
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

		InitParmas();
		CreateGUI();
		LoadAssets();

		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
		DemoBase::Release();

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

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();
		UpdateUI(time, delta);

		m_ModelScene->rootNode->localMatrix.AppendRotation(delta * 90.0f, Vector3::UpVector);
		for (int32 i = 0; i < m_SceneMatMeshes.size(); ++i)
		{
			m_SceneMaterials[i]->BeginFrame();
			for (int32 j = 0; j < m_SceneMatMeshes[i].size(); ++j) {
				m_MVPData.model = m_SceneMatMeshes[i][j]->linkNode->GetGlobalMatrix();
				m_SceneMaterials[i]->BeginObject();
				m_SceneMaterials[i]->SetLocalUniform("uboMVP", &m_MVPData, sizeof(ModelViewProjectionBlock));
				m_SceneMaterials[i]->EndObject();
			}
			m_SceneMaterials[i]->EndFrame();
		}

		SetupCommandBuffers(bufferIndex);
		DemoBase::Present(bufferIndex);
	}

	void UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("InstanceDrawDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		m_GUI->EndFrame();
		m_GUI->Update();
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// room model
		m_ModelScene = vk_demo::DVKModel::LoadFromFile(
			"assets/models/Room/miniHouse_FBX.FBX",
			m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal }
		);

		// room shader
		m_SceneShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/24_EdgeDetect/obj.vert.spv",
			"assets/shaders/24_EdgeDetect/obj.frag.spv"
		);

		// Room textures
		std::vector<std::string> diffusePaths = {
			"assets/models/Room/miniHouse_Part1.jpg",
			"assets/models/Room/miniHouse_Part2.jpg",
			"assets/models/Room/miniHouse_Part3.jpg",
			"assets/models/Room/miniHouse_Part4.jpg"
		};
		m_SceneDiffuses.resize(diffusePaths.size());
		for (int32 i = 0; i < diffusePaths.size(); ++i)
		{
			m_SceneDiffuses[i] = vk_demo::DVKTexture::Create2D(
				diffusePaths[i],
				m_VulkanDevice,
				cmdBuffer
			);
		}

		// room material
		m_SceneMaterials.resize(m_SceneDiffuses.size());
		for (int32 i = 0; i < m_SceneMaterials.size(); ++i)
		{
			m_SceneMaterials[i] = vk_demo::DVKMaterial::Create(
				m_VulkanDevice,
				m_RenderPass,
				m_PipelineCache,
				m_SceneShader
			);
			m_SceneMaterials[i]->PreparePipeline();
			m_SceneMaterials[i]->SetTexture("diffuseMap", m_SceneDiffuses[i]);
		}

		// collect meshles
		m_SceneMatMeshes.resize(m_SceneDiffuses.size());
		for (int32 i = 0; i < m_ModelScene->meshes.size(); ++i)
		{
			vk_demo::DVKMesh* mesh = m_ModelScene->meshes[i];
			const std::string& diffuseName = mesh->material.diffuse;
			if (diffuseName == "miniHouse_Part1") {
				m_SceneMatMeshes[0].push_back(mesh);
			}
			else if (diffuseName == "miniHouse_Part2") {
				m_SceneMatMeshes[1].push_back(mesh);
			}
			else if (diffuseName == "miniHouse_Part3") {
				m_SceneMatMeshes[2].push_back(mesh);
			}
			else if (diffuseName == "miniHouse_Part4") {
				m_SceneMatMeshes[3].push_back(mesh);
			}
		}

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
        delete m_SkyModel;
        delete m_SkyShader;
        delete m_SkyMaterial;
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

			for (int32 i = 0; i < m_SceneMatMeshes.size(); ++i)
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SceneMaterials[i]->GetPipeline());
				for (int32 j = 0; j < m_SceneMatMeshes[i].size(); ++j) {
					m_SceneMaterials[i]->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
					m_SceneMatMeshes[i][j]->BindDrawCmd(commandBuffer);
				}
			}

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_MVPData.model.SetIdentity();
		m_MVPData.model.SetOrigin(Vector3(0, 0, 0));

		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector3(0, 100.0f, -750.0f));
		m_MVPData.view.AppendRotation(22.50f, Vector3::RightVector);
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(MMath::DegreesToRadians(75.0f), (float)GetWidth(), (float)GetHeight(), 10.0f, 3000.0f);
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
    
	bool 						m_Ready = false;

	ModelViewProjectionBlock	m_MVPData;

	vk_demo::DVKModel*			m_SkyModel = nullptr;
	vk_demo::DVKShader*			m_SkyShader = nullptr;
    vk_demo::DVKMaterial*       m_SkyMaterial = nullptr;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<InstanceDrawDemo>(1400, 900, "InstanceDrawDemo", cmdLine);
}
