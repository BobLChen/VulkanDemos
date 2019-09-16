#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class SimpleTessellationDemo : public DemoBase
{
public:
	SimpleTessellationDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~SimpleTessellationDemo()
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
		Matrix4x4 proj;
	};

	struct TessParamBlock
	{
		Vector4 levelOuter;
		Vector4 levelInner;
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
			ImGui::Begin("SimpleTessellationDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			float level = m_VulkanDevice->GetLimits().maxTessellationGenerationLevel;

			ImGui::Separator();
			ImGui::SliderFloat2("LevelInner:", (float*)&(m_TessParam.levelInner), 0.0f, level);
			ImGui::Separator();
			ImGui::SliderFloat4("LevelOuter:", (float*)&(m_TessParam.levelOuter), 0.0f, level);

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

		m_PatchTriangle = vk_demo::DVKModel::Create(
			m_VulkanDevice,
			cmdBuffer,
			{
				-10,  10,  0.0f,
				 10,  10,  0.0f,
				 10, -10,  0.0f,
				-10, -10,  0.0f
			},
			{ 0, 1, 2, 0, 2, 3 },
			{ VertexAttribute::VA_Position }
		);

		m_PatchQuat = vk_demo::DVKModel::Create(
			m_VulkanDevice,
			cmdBuffer,
			{
				-10,  10,  0.0f,
				 10,  10,  0.0f,
				 10, -10,  0.0f,
				-10, -10,  0.0f
			},
			{ 0, 1, 2, 3 },
			{ VertexAttribute::VA_Position }
		);

		m_PatchIso = vk_demo::DVKModel::Create(
			m_VulkanDevice,
			cmdBuffer,
			{
				-10,  10,  0.0f,
				 10,  10,  0.0f,
				 10, -10,  0.0f,
				-10, -10,  0.0f
			},
			{ 0, 1, 2, 3 },
			{ VertexAttribute::VA_Position }
		);

		m_ShaderTri = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/49_SimpleTessellation/Simple.vert.spv",
			"assets/shaders/49_SimpleTessellation/Simple.frag.spv",
			nullptr,
			nullptr,
			"assets/shaders/49_SimpleTessellation/SimpleTri.tesc.spv",
			"assets/shaders/49_SimpleTessellation/SimpleTri.tese.spv"
		);

		m_MaterialTri = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_ShaderTri
		);
		m_MaterialTri->pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_NONE;
		m_MaterialTri->pipelineInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		m_MaterialTri->pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		m_MaterialTri->pipelineInfo.tessellationState.patchControlPoints = 3;
		m_MaterialTri->PreparePipeline();

		m_ShaderQuad = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/49_SimpleTessellation/Simple.vert.spv",
			"assets/shaders/49_SimpleTessellation/Simple.frag.spv",
			nullptr,
			nullptr,
			"assets/shaders/49_SimpleTessellation/SimpleQuad.tesc.spv",
			"assets/shaders/49_SimpleTessellation/SimpleQuad.tese.spv"
		);

		m_MaterialQuat = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_ShaderQuad
		);
		m_MaterialQuat->pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_NONE;
		m_MaterialQuat->pipelineInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		m_MaterialQuat->pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		m_MaterialQuat->pipelineInfo.tessellationState.patchControlPoints = 4;
		m_MaterialQuat->PreparePipeline();

		m_ShaderIso = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/49_SimpleTessellation/Simple.vert.spv",
			"assets/shaders/49_SimpleTessellation/Simple.frag.spv",
			nullptr,
			nullptr,
			"assets/shaders/49_SimpleTessellation/SimpleIso.tesc.spv",
			"assets/shaders/49_SimpleTessellation/SimpleIso.tese.spv"
		);

		m_MaterialIso = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_ShaderIso
		);
		m_MaterialIso->pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_NONE;
		m_MaterialIso->pipelineInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		m_MaterialIso->pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		m_MaterialIso->pipelineInfo.tessellationState.patchControlPoints = 2;
		m_MaterialIso->PreparePipeline();

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_PatchTriangle;
		delete m_PatchQuat;
		delete m_PatchIso;

		delete m_ShaderTri;
		delete m_ShaderQuad;
		delete m_ShaderIso;

		delete m_MaterialTri;
		delete m_MaterialQuat;
		delete m_MaterialIso;
	}

	void DrawModel(VkCommandBuffer commandBuffer, vk_demo::DVKModel* model, vk_demo::DVKMaterial* material)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->GetPipeline());

		material->BeginFrame();
		for (int32 i = 0; i < model->meshes.size(); ++i)
		{
			m_MVPParam.model = model->meshes[i]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = m_ViewCamera.GetView();
			m_MVPParam.proj  = m_ViewCamera.GetProjection();

			material->BeginObject();
			material->SetLocalUniform("tessParam", &m_TessParam, sizeof(TessParamBlock));
			material->SetLocalUniform("uboMVP", &m_MVPParam, sizeof(ModelViewProjectionBlock));
			material->EndObject();

			material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, i);
			model->meshes[i]->BindDrawCmd(commandBuffer);
		}
		material->EndFrame();
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		float hh = m_FrameHeight * 0.5f;
		float hw = m_FrameWidth  * 0.5f;

		VkViewport viewport = {};
		viewport.x        = 0;
		viewport.y        = 0;
		viewport.width    = hw;
		viewport.height   = -(float)hh;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = hw;
		scissor.extent.height = hh;
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

		{
			viewport.y = hh;
			viewport.x = hw;
			scissor.offset.x = hw;
			scissor.offset.y = 0;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			DrawModel(commandBuffer, m_PatchTriangle, m_MaterialTri);
		}
		
		{
			viewport.y = hh + hh;
			viewport.x = 0;
			scissor.offset.x = 0;
			scissor.offset.y = hh;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			DrawModel(commandBuffer, m_PatchQuat, m_MaterialQuat);
		}

		{
			viewport.y = hh + hh;
			viewport.x = hw;
			scissor.offset.x = hw;
			scissor.offset.y = hh;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			DrawModel(commandBuffer, m_PatchIso, m_MaterialIso);
		}

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);
		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_TessParam.levelInner.Set(1, 1, 1, 1);
		m_TessParam.levelOuter.Set(2, 2, 2, 2);

		m_ViewCamera.SetPosition(0, 0, -50.0f);
		m_ViewCamera.LookAt(0, 0, 0);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 1.0f, 1500.0f);
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

	bool 						    m_Ready = false;

	vk_demo::DVKModel*			    m_PatchTriangle = nullptr;
	vk_demo::DVKModel*			    m_PatchQuat = nullptr;
	vk_demo::DVKModel*			    m_PatchIso = nullptr;

	vk_demo::DVKShader*			    m_ShaderTri = nullptr;
	vk_demo::DVKShader*			    m_ShaderQuad = nullptr;
	vk_demo::DVKShader*			    m_ShaderIso = nullptr;

	vk_demo::DVKMaterial*		    m_MaterialTri = nullptr;
	vk_demo::DVKMaterial*		    m_MaterialQuat = nullptr;
	vk_demo::DVKMaterial*		    m_MaterialIso = nullptr;

	vk_demo::DVKCamera		        m_ViewCamera;
	ModelViewProjectionBlock	    m_MVPParam;
	TessParamBlock					m_TessParam;

	ImageGUIContext*			    m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<SimpleTessellationDemo>(1400, 900, "SimpleTessellationDemo", cmdLine);
}
