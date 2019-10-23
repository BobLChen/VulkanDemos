#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"

#include <vector>

class StencilDemo : public DemoBase
{
public:
	StencilDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~StencilDemo()
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

	struct RayParamBlock
	{
		Vector3    color;
		float	   power;
		Vector3    viewDir;
		float	   padding;
	};

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		UpdateUniform(time, delta);

		// 设置Role参数
		m_RoleMaterial->BeginFrame();
		for (int32 i = 0; i < m_ModelRole->meshes.size(); ++i) {
			m_MVPData.model = m_ModelRole->meshes[i]->linkNode->GetGlobalMatrix();
			m_RoleMaterial->BeginObject();
			m_RoleMaterial->SetLocalUniform("uboMVP", &m_MVPData, sizeof(ModelViewProjectionBlock));
			m_RoleMaterial->EndObject();
		}
		m_RoleMaterial->EndFrame();

		// ray参数
		m_RayMaterial->BeginFrame();
		for (int32 i = 0; i < m_ModelRole->meshes.size(); ++i) {
			m_MVPData.model = m_ModelRole->meshes[i]->linkNode->GetGlobalMatrix();
			m_RayMaterial->BeginObject();
			m_RayMaterial->SetLocalUniform("uboMVP",   &m_MVPData, sizeof(ModelViewProjectionBlock));
			m_RayMaterial->SetLocalUniform("rayParam", &m_RayData, sizeof(RayParamBlock));
			m_RayMaterial->EndObject();
		}
		m_RayMaterial->EndFrame();

		// 设置Room参数
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

	void UpdateUniform(float time, float delta)
	{
		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();

		m_RayData.viewDir = -m_ViewCamera.GetTransform().GetForward();
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("StencilDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			{
				Vector3 position = m_ModelRole->rootNode->localMatrix.GetOrigin();
				Vector3 scale = m_ModelRole->rootNode->localMatrix.GetScaleVector();
				ImGui::SliderFloat3("Position", (float*)&position, -500.0f, 500.0f);
				ImGui::SliderFloat3("Scale", (float*)&scale, 1.0f, 100.0f);
				m_ModelRole->rootNode->localMatrix.SetIdentity();
				m_ModelRole->rootNode->localMatrix.AppendScale(scale);
				m_ModelRole->rootNode->localMatrix.AppendTranslation(position);
			}

			{
				ImGui::ColorEdit3("Color", (float*)&m_RayData.color);
				ImGui::SliderFloat("Pow", &m_RayData.power, 1.0f, 10.0f);
			}

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
		// diffuse shader
		m_DiffuseShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/21_Stencil/obj.vert.spv",
			"assets/shaders/21_Stencil/obj.frag.spv"
		);

		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// Role Model
		m_ModelRole = vk_demo::DVKModel::LoadFromFile(
			"assets/models/LizardMage/LizardMage_Lowpoly.obj",
			m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal }
		);
		m_ModelRole->rootNode->localMatrix.AppendScale(Vector3(100.0f, 100.0f, 100.0f));
		m_ModelRole->rootNode->localMatrix.AppendTranslation(Vector3(-15.0f, 300.0f, 500.0f));
		// Role diffuse
		m_RoleDiffuse = vk_demo::DVKTexture::Create2D(
			"assets/models/LizardMage/Body_colors1.jpg",
			m_VulkanDevice,
			cmdBuffer
		);

		// Room Model
		m_ModelScene = vk_demo::DVKModel::LoadFromFile(
			"assets/models/Room/miniHouse_FBX.FBX",
			m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal }
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
		
		delete cmdBuffer;

		// role material
		m_RoleMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_DiffuseShader
		);
		m_RoleMaterial->PreparePipeline();
		m_RoleMaterial->SetTexture("diffuseMap", m_RoleDiffuse);
		// room material
		m_SceneMaterials.resize(m_SceneDiffuses.size());
		for (int32 i = 0; i < m_SceneMaterials.size(); ++i)
		{
			m_SceneMaterials[i] = vk_demo::DVKMaterial::Create(
				m_VulkanDevice,
				m_RenderPass,
				m_PipelineCache,
				m_DiffuseShader
			);
			VkPipelineDepthStencilStateCreateInfo& depthStencilState = m_SceneMaterials[i]->pipelineInfo.depthStencilState;
			depthStencilState.stencilTestEnable = VK_TRUE;
			depthStencilState.back.reference    = 1;
			depthStencilState.back.writeMask    = 0xFF;
			depthStencilState.back.compareMask  = 0xFF;
			depthStencilState.back.compareOp    = VK_COMPARE_OP_ALWAYS;
			depthStencilState.back.failOp       = VK_STENCIL_OP_REPLACE;
			depthStencilState.back.depthFailOp  = VK_STENCIL_OP_REPLACE;
			depthStencilState.back.passOp       = VK_STENCIL_OP_REPLACE;
			depthStencilState.front             = depthStencilState.back;
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

		// ray effect
		m_RayShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/21_Stencil/ray.vert.spv",
			"assets/shaders/21_Stencil/ray.frag.spv"
		);
		// ray material
		m_RayMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_RayShader
		);
		VkPipelineColorBlendAttachmentState& blendState = m_RayMaterial->pipelineInfo.blendAttachmentStates[0];
		blendState.blendEnable         = VK_TRUE;
		blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendState.colorBlendOp        = VK_BLEND_OP_ADD;
        
		VkPipelineDepthStencilStateCreateInfo& depthStencilState = m_RayMaterial->pipelineInfo.depthStencilState;
		depthStencilState.stencilTestEnable = VK_TRUE;
		depthStencilState.back.reference    = 1;
		depthStencilState.back.writeMask    = 0xFF;
		depthStencilState.back.compareMask  = 0xFF;
		depthStencilState.back.compareOp    = VK_COMPARE_OP_EQUAL;
		depthStencilState.back.failOp       = VK_STENCIL_OP_KEEP;
		depthStencilState.back.depthFailOp  = VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp       = VK_STENCIL_OP_REPLACE;
		depthStencilState.front             = depthStencilState.back;
        depthStencilState.depthTestEnable   = VK_FALSE;
		m_RayMaterial->PreparePipeline();
	}
    
	void DestroyAssets()
	{
		delete m_DiffuseShader;

		delete m_ModelRole;
		delete m_RoleDiffuse;
		delete m_RoleMaterial;

		delete m_ModelScene;
		for (int32 i = 0; i < m_SceneDiffuses.size(); ++i) {
			delete m_SceneDiffuses[i];
		}
		m_SceneDiffuses.clear();

		for (int32 i = 0; i < m_SceneMaterials.size(); ++i) {
			delete m_SceneMaterials[i];
		}
		m_SceneMaterials.clear();

		delete m_RayShader;
		delete m_RayMaterial;
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color = { { 0.2f, 0.2f, 0.2f, 0.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass  = m_RenderPass;
		renderPassBeginInfo.framebuffer = m_FrameBuffers[backBufferIndex];
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues    = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
		renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = m_FrameHeight;
		viewport.width  = m_FrameWidth;
		viewport.height = -(float)m_FrameHeight;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = m_FrameWidth;
		scissor.extent.height = m_FrameHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));
		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

		// role
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RoleMaterial->GetPipeline());
        for (int32 meshIndex = 0; meshIndex < m_ModelRole->meshes.size(); ++meshIndex) {
            m_RoleMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, meshIndex);
            m_ModelRole->meshes[meshIndex]->BindDrawCmd(commandBuffer);
        }

		// room
		for (int32 i = 0; i < m_SceneMatMeshes.size(); ++i)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SceneMaterials[i]->GetPipeline());
			for (int32 j = 0; j < m_SceneMatMeshes[i].size(); ++j) {
				m_SceneMaterials[i]->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
				m_SceneMatMeshes[i][j]->BindDrawCmd(commandBuffer);
			}
		}

		// ray
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RayMaterial->GetPipeline());
		for (int32 meshIndex = 0; meshIndex < m_ModelRole->meshes.size(); ++meshIndex) {
			m_RayMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, meshIndex);
			m_ModelRole->meshes[meshIndex]->BindDrawCmd(commandBuffer);
		}

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass, 0);

		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 10.0f, 5000.0f);
		m_ViewCamera.SetPosition(0, 500.0f, -1500.0f);
		m_ViewCamera.LookAt(0, 0, 0);

		m_RayData.viewDir = -m_ViewCamera.GetTransform().GetForward();
		m_RayData.color   = Vector3(0.0f, 0.6f, 1.0f);
		m_RayData.power   = 5.0f;
		m_RayData.padding = 0;
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

	bool 							m_Ready = false;

	vk_demo::DVKCamera				m_ViewCamera;

	ModelViewProjectionBlock		m_MVPData;
	RayParamBlock					m_RayData;
	
	vk_demo::DVKShader*				m_DiffuseShader = nullptr;

	vk_demo::DVKModel*				m_ModelRole = nullptr;
	vk_demo::DVKTexture*			m_RoleDiffuse = nullptr;
	vk_demo::DVKMaterial*			m_RoleMaterial = nullptr;

	vk_demo::DVKModel*				m_ModelScene = nullptr;
	TextureArray					m_SceneDiffuses;
	MaterialArray					m_SceneMaterials;
	MatMeshArray					m_SceneMatMeshes;

	vk_demo::DVKShader*				m_RayShader = nullptr;
	vk_demo::DVKMaterial*			m_RayMaterial = nullptr;

	ImageGUIContext*				m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<StencilDemo>(1400, 900, "StencilDemo", cmdLine);
}
