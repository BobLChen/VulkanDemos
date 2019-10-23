#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

// http://yangwc.com/2019/07/14/PhysicallyBasedRendering/
class PBRDirectLightingDemo : public DemoBase
{
public:
	PBRDirectLightingDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~PBRDirectLightingDemo()
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

	struct PBRParamBlock
	{
		Vector4 param;
		Vector4 cameraPos;
		Vector4 lightColor;
	};

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		m_PBRParam.cameraPos = m_ViewCamera.GetTransform().GetOrigin();

		SetupCommandBuffers(bufferIndex);
		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("PBRDirectLightingDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::SliderFloat("AO", &m_PBRParam.param.x, 0.0f, 1.0f);
			ImGui::SliderFloat("Roughness", &m_PBRParam.param.y, 0.0f, 1.0f);
			ImGui::SliderFloat("Metallic", &m_PBRParam.param.z, 0.0f, 1.0f);

			ImGui::Separator();

			ImGui::ColorEdit3("LightColor", (float*)(&m_PBRParam.lightColor));
			ImGui::SliderFloat("Intensity", &m_PBRParam.lightColor.w, 0.0f, 100.0f);

			ImGui::Separator();

			int32 debug = m_PBRParam.param.w;
			const char* models[6] = {
				"None",
				"Albedo",
				"Normal",
				"Occlusion",
				"Metallic",
				"Roughness"
			};
			ImGui::Combo("Debug", &debug, models, 6);
			m_PBRParam.param.w = debug;

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/leather-shoes/model.fbx",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position,
				VertexAttribute::VA_UV0,
				VertexAttribute::VA_Normal,
				VertexAttribute::VA_Tangent
			}
		);
		m_Model->rootNode->localMatrix.AppendRotation(180, Vector3::UpVector);

		m_TexAlbedo = vk_demo::DVKTexture::Create2D(
			"assets/models/leather-shoes/RootNode_baseColor.jpg",
			m_VulkanDevice,
			cmdBuffer
		);

		m_TexNormal = vk_demo::DVKTexture::Create2D(
			"assets/models/leather-shoes/RootNode_normal.jpg",
			m_VulkanDevice,
			cmdBuffer
		);

		m_TexORMParam = vk_demo::DVKTexture::Create2D(
			"assets/models/leather-shoes/RootNode_occlusionRoughnessMetallic.jpg",
			m_VulkanDevice,
			cmdBuffer
		);

		m_Shader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/55_PBR_DirectLighting/obj.vert.spv",
			"assets/shaders/55_PBR_DirectLighting/obj.frag.spv"
		);

		m_Material = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader
		);
		m_Material->PreparePipeline();
		m_Material->SetTexture("texAlbedo", m_TexAlbedo);
		m_Material->SetTexture("texNormal", m_TexNormal);
		m_Material->SetTexture("texORMParam", m_TexORMParam);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;

		delete m_Shader;
		delete m_Material;

		delete m_TexAlbedo;
		delete m_TexNormal;
		delete m_TexORMParam;
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

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

		for (int32 i = 0; i < m_Model->meshes.size(); ++i)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());

			m_Material->BeginFrame();

			m_MVPParam.model = m_Model->meshes[i]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = m_ViewCamera.GetView();
			m_MVPParam.proj  = m_ViewCamera.GetProjection();

			m_Material->BeginObject();
			m_Material->SetLocalUniform("uboMVP",   &m_MVPParam, sizeof(ModelViewProjectionBlock));
			m_Material->SetLocalUniform("uboParam", &m_PBRParam, sizeof(PBRParamBlock));
			m_Material->EndObject();

			m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_Model->meshes[i]->BindDrawCmd(commandBuffer);

			m_Material->EndFrame();
		}

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		m_ViewCamera.SetPosition(boundCenter.x, boundCenter.y, boundCenter.z - 500.0f);
		m_ViewCamera.LookAt(boundCenter);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 1.0f, 3000.0f);

		m_PBRParam.param.x = 1.0f; // ao
		m_PBRParam.param.y = 1.0f; // roughness
		m_PBRParam.param.z = 1.0f; // metallic
		m_PBRParam.param.w = 0.0f; // debug

		m_PBRParam.cameraPos = m_ViewCamera.GetTransform().GetOrigin();

		m_PBRParam.lightColor = Vector4(1, 1, 1, 10);
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

	vk_demo::DVKModel*			m_Model = nullptr;
	vk_demo::DVKShader*			m_Shader = nullptr;
	vk_demo::DVKMaterial*		m_Material = nullptr;

	vk_demo::DVKTexture*		m_TexAlbedo = nullptr;
	vk_demo::DVKTexture*		m_TexNormal = nullptr;
	vk_demo::DVKTexture*		m_TexORMParam = nullptr;

	vk_demo::DVKCamera		    m_ViewCamera;

	ModelViewProjectionBlock	m_MVPParam;
	PBRParamBlock				m_PBRParam;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<PBRDirectLightingDemo>(1400, 900, "PBRDirectLightingDemo", cmdLine);
}
