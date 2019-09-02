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

#define OBJECT_COUNT 1024

class ComputeFrustumDemo : public DemoBase
{
public:
	ComputeFrustumDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~ComputeFrustumDemo()
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
			UpdateFrustumPlanes();
		}

		SetupGfxCommand(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("ComputeFrustumDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

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

		m_ModelSphere = vk_demo::DVKModel::LoadFromFile(
			"assets/models/sphere.obj",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position, 
				VertexAttribute::VA_Normal
			}
		);
		auto bounds = m_ModelSphere->rootNode->GetBounds();
		m_Radius = bounds.max.x - bounds.min.x;

		for (int32 i = 0; i < OBJECT_COUNT; ++i)
		{
			m_ObjModels[i].AppendTranslation(Vector3(
				MMath::FRandRange(-450.0f, 450.0f),
				19.73f,
				MMath::FRandRange(-450.0f, 450.0f)
			));
		}

		m_Shader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/39_OcclusionQueries/Solid.vert.spv",
			"assets/shaders/39_OcclusionQueries/Solid.frag.spv"
		);

		m_Material = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader
		);
		m_Material->PreparePipeline();

		m_LineShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/39_OcclusionQueries/Line.vert.spv",
			"assets/shaders/39_OcclusionQueries/Line.frag.spv"
		);

		m_LineMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_LineShader
		);
		m_LineMaterial->pipelineInfo.inputAssemblyState.topology    = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		m_LineMaterial->pipelineInfo.rasterizationState.cullMode    = VK_CULL_MODE_NONE;
		m_LineMaterial->pipelineInfo.rasterizationState.lineWidth   = 1.0f;
		m_LineMaterial->pipelineInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		m_LineMaterial->PreparePipeline();

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_ModelSphere;

		delete m_LineShader;
		delete m_LineMaterial;

		delete m_Material;
		delete m_Shader;
	}

	bool IsInFrustum(const Matrix4x4& model)
	{
		Vector3 pos  = model.GetOrigin();
		
		for (int32 i = 0; i < 6; ++i) 
		{
			Vector4& plane = m_FrustumPlanes[i];
			Vector3 center = model.GetOrigin();
			float projDist = (plane.x * pos.x) + (plane.y * pos.y) + (plane.z * pos.z) + plane.w + m_Radius;
			if (projDist <= 0) {
				return false;
			}
		}

		return true;
	}

	void RenderSpheres(VkCommandBuffer commandBuffer, vk_demo::DVKCamera& camera)
	{
		m_Material->BeginFrame();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());
		m_ModelSphere->meshes[0]->BindOnly(commandBuffer);
		
		int32 count = 0;
		for (int32 i = 0; i < OBJECT_COUNT; ++i)
		{
			if (IsInFrustum(m_ObjModels[i])) 
			{
				m_MVPParam.model = m_ObjModels[i];
				m_MVPParam.view  = camera.GetView();
				m_MVPParam.proj  = camera.GetProjection();

				m_Material->BeginObject();
				m_Material->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
				m_Material->EndObject();

				m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, count);
				m_ModelSphere->meshes[0]->DrawOnly(commandBuffer);

				count++;
			}
		}

		m_Material->EndFrame();
	}

	void SetupGfxCommand(int32 backBufferIndex)
	{
		VkViewport viewport = {};
		viewport.x        = 0;
		viewport.y        = m_FrameHeight;
		viewport.width    = m_FrameWidth;
		viewport.height   = -(float)m_FrameHeight / 2;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = m_FrameWidth;
		scissor.extent.height = m_FrameHeight / 2;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

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

		// normal
		{
			viewport.y = m_FrameHeight * 0.5f;
			scissor.offset.y = 0;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			RenderSpheres(commandBuffer, m_ViewCamera);
		}
		
		// occlusion view
		{
			viewport.y = m_FrameHeight;
			scissor.offset.y = m_FrameHeight * 0.5f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			RenderSpheres(commandBuffer, m_TopCamera);
		}
		
		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void UpdateFrustumPlanes()
	{
		Matrix4x4 matrix = m_ViewCamera.GetViewProjection();

		// left
		m_FrustumPlanes[0].x = matrix.m[0][3] + matrix.m[0][0];
		m_FrustumPlanes[0].y = matrix.m[1][3] + matrix.m[1][0];
		m_FrustumPlanes[0].z = matrix.m[2][3] + matrix.m[2][0];
		m_FrustumPlanes[0].w = matrix.m[3][3] + matrix.m[3][0];

		// right
		m_FrustumPlanes[1].x = matrix.m[0][3] - matrix.m[0][0];
		m_FrustumPlanes[1].y = matrix.m[1][3] - matrix.m[1][0];
		m_FrustumPlanes[1].z = matrix.m[2][3] - matrix.m[2][0];
		m_FrustumPlanes[1].w = matrix.m[3][3] - matrix.m[3][0];

		// top
		m_FrustumPlanes[2].x = matrix.m[0][3] + matrix.m[0][1];
		m_FrustumPlanes[2].y = matrix.m[1][3] + matrix.m[1][1];
		m_FrustumPlanes[2].z = matrix.m[2][3] + matrix.m[2][1];
		m_FrustumPlanes[2].w = matrix.m[3][3] + matrix.m[3][1];

		// bottom
		m_FrustumPlanes[3].x = matrix.m[0][3] - matrix.m[0][1];
		m_FrustumPlanes[3].y = matrix.m[1][3] - matrix.m[1][1];
		m_FrustumPlanes[3].z = matrix.m[2][3] - matrix.m[2][1];
		m_FrustumPlanes[3].w = matrix.m[3][3] - matrix.m[3][1];

		// near
		m_FrustumPlanes[4].x = matrix.m[0][2];
		m_FrustumPlanes[4].y = matrix.m[1][2];
		m_FrustumPlanes[4].z = matrix.m[2][2];
		m_FrustumPlanes[4].w = matrix.m[3][2];

		// far
		m_FrustumPlanes[5].x = matrix.m[0][3] - matrix.m[0][2];
		m_FrustumPlanes[5].y = matrix.m[1][3] - matrix.m[1][2];
		m_FrustumPlanes[5].z = matrix.m[2][3] - matrix.m[2][2];
		m_FrustumPlanes[5].w = matrix.m[3][3] - matrix.m[3][2];

		for (auto i = 0; i < 6; i++)
		{
			float length = MMath::Sqrt(m_FrustumPlanes[i].x * m_FrustumPlanes[i].x + m_FrustumPlanes[i].y * m_FrustumPlanes[i].y + m_FrustumPlanes[i].z * m_FrustumPlanes[i].z);
			m_FrustumPlanes[i].x /= length;
			m_FrustumPlanes[i].y /= length;
			m_FrustumPlanes[i].z /= length;
			m_FrustumPlanes[i].w /= length;
		}
	}

	void InitParmas()
	{
		m_ViewCamera.freeze.x = 1;
		m_ViewCamera.SetPosition(0, 19.73f, -800.0f);
		m_ViewCamera.LookAt(0, 19.73f, 0);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight() * 0.5f, 1.0f, 1500.0f);

		m_TopCamera.SetPosition(-500, 1500, 0);
		m_TopCamera.LookAt(0, 0, 0);
		m_TopCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight() * 0.5f, 1.0f, 3000.0f);
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

	typedef std::vector<vk_demo::DVKTexture*>			TextureArray;
	typedef std::vector<vk_demo::DVKMaterial*>			MaterialArray;
	typedef std::vector<std::vector<vk_demo::DVKMesh*>> MatMeshArray;

	bool 						m_Ready = false;

	vk_demo::DVKModel*			m_ModelSphere = nullptr;

	vk_demo::DVKMaterial*		m_Material = nullptr;
	vk_demo::DVKShader*			m_Shader = nullptr;

	vk_demo::DVKShader*			m_LineShader = nullptr;
	vk_demo::DVKMaterial*		m_LineMaterial = nullptr;

	Matrix4x4					m_ObjModels[OBJECT_COUNT];

	vk_demo::DVKCamera		    m_ViewCamera;
	vk_demo::DVKCamera			m_TopCamera;

	ModelViewProjectionBlock	m_MVPParam;
	Vector4						m_FrustumPlanes[6];
	float						m_Radius;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<ComputeFrustumDemo>(1400, 900, "ComputeFrustumDemo", cmdLine);
}
