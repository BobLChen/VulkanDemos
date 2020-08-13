#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Math/Quat.h"

#include <vector>

class ShuffleIntrinsicsDemo : public DemoBase
{
public:
	ShuffleIntrinsicsDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
		deviceExtensions.push_back(VK_EXT_SHADER_SUBGROUP_BALLOT_EXTENSION_NAME);
		deviceExtensions.push_back(VK_EXT_SHADER_SUBGROUP_VOTE_EXTENSION_NAME);
	}

	virtual ~ShuffleIntrinsicsDemo()
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
		InitParmas();
		CreateGUI();

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

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateUI(time, delta);

		m_Material->BeginFrame();
		m_Material->BeginObject();
		m_Material->SetLocalUniform("uboParam",    &m_Type,   sizeof(Vector4));
		m_Material->EndObject();
		m_Material->EndFrame();

		SetupCommandBuffers(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("ShuffleIntrinsicsDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			int type = m_Type.x;
			ImGui::Combo("Type", &type, "None\0SubGroupInvocation\0WarpID\0SMID\0FirstInvocation\0\0");
			m_Type.x = type;

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

		// model
		m_Model = vk_demo::DVKDefaultRes::fullQuad;

		// shader
		m_Shader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/71_ShuffleIntrinsics/shuffle.vert.spv",
			"assets/shaders/71_ShuffleIntrinsics/shuffle.frag.spv"
		);

		// material
		m_Material = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader
		);
		m_Material->PreparePipeline();

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Shader;
		delete m_Material;
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
		for (int32 j = 0; j < m_Model->meshes.size(); ++j) {
			m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
			m_Model->meshes[j]->BindDrawCmd(commandBuffer);
		}

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_Type.Set(1, 1, 1, 1);
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
	vk_demo::DVKMaterial*       m_Material = nullptr;

	Vector4						m_Type;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<ShuffleIntrinsicsDemo>(1400, 900, "ShuffleIntrinsicsDemo", cmdLine);
}
