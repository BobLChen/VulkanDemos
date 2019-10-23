#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class BloomDemo : public DemoBase
{
public:
	BloomDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~BloomDemo()
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
		CreateRenderTarget();
		CreateGUI();
		LoadAssets();

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
    
    struct FilterParamBlock
    {
        float width;
        float height;
        float step;
        float bright;
    };
    
	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();

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

		m_BrightMaterial->SetGlobalUniform("param", &m_FilterParam, sizeof(FilterParamBlock));
		m_BrightMaterial->BeginFrame();
		m_BrightMaterial->EndFrame();
        
		m_BlurHMaterial->SetGlobalUniform("param", &m_FilterParam, sizeof(FilterParamBlock));
		m_BlurHMaterial->BeginFrame();
		m_BlurHMaterial->EndFrame();
		
		m_BlurVMateria->SetGlobalUniform("param", &m_FilterParam, sizeof(FilterParamBlock));
		m_BlurVMateria->BeginFrame();
		m_BlurVMateria->EndFrame();

		SetupCommandBuffers(bufferIndex);
		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("BloomDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            
			ImGui::SliderFloat("BlurStep", &m_FilterParam.step, 1.0f, 2.0f);
			ImGui::SliderFloat("Bright", &m_FilterParam.bright, 0.5f, 0.9f);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void CreateRenderTarget()
	{
		m_RTColor = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(GetVulkanRHI()->GetPixelFormat(), false), 
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
        
		m_RTColorQuater0 = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(GetVulkanRHI()->GetPixelFormat(), false), 
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth / 4.0f, m_FrameHeight / 4.0f,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
        
		m_RTColorQuater1 = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(GetVulkanRHI()->GetPixelFormat(), false), 
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth / 4.0f, m_FrameHeight / 4.0f,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
        
		m_RTDepth = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);
        
		// 正常渲染场景
		vk_demo::DVKRenderPassInfo rttNormalInfo(
			m_RTColor, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			m_RTDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
		);
		m_RTTNormal = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttNormalInfo);

		// 1/4降级渲染
		vk_demo::DVKRenderPassInfo rttQuater0Info(
			m_RTColorQuater0, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, nullptr
		);
		m_RTTQuater0 = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttQuater0Info);

		// 1/4降级渲染
		vk_demo::DVKRenderPassInfo rttQuater1Info(
			m_RTColorQuater1, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, nullptr
		);
		m_RTTQuater1 = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttQuater1Info);

	}

	void DestroyRenderTarget()
	{
		delete m_RTTNormal;
		delete m_RTTQuater0;
		delete m_RTTQuater1;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// quad model
		m_Quad = vk_demo::DVKDefaultRes::fullQuad;

		// room model
		m_ModelScene = vk_demo::DVKModel::LoadFromFile(
			"assets/models/Blacksmith/tiejiang.fbx",
			m_VulkanDevice,
			cmdBuffer,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal }
		);
		m_ModelScene->rootNode->localMatrix.AppendRotation(180, Vector3::UpVector);

		// room shader
		m_SceneShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/25_Bloom/obj.vert.spv",
			"assets/shaders/25_Bloom/obj.frag.spv"
		);
        
		// Room textures
		std::vector<std::string> diffusePaths = {
			"assets/models/Blacksmith/Anvil_Tex.jpg",
			"assets/models/Blacksmith/BLACKSMITH_TEX.jpg",
			"assets/models/Blacksmith/FloorTex.jpg",
			"assets/models/Blacksmith/Spark_Diff.jpg"
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

		// room materials
		m_SceneMaterials.resize(m_SceneDiffuses.size());
		for (int32 i = 0; i < m_SceneMaterials.size(); ++i)
		{
			m_SceneMaterials[i] = vk_demo::DVKMaterial::Create(
				m_VulkanDevice,
				m_RTTNormal,
				m_PipelineCache,
				m_SceneShader
			);
			// 最后一个叠加混合
			if (i + 1 == m_SceneMaterials.size()) {
				m_SceneMaterials[i]->pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_NONE;

				VkPipelineColorBlendAttachmentState& blendAttachmentState = m_SceneMaterials[i]->pipelineInfo.blendAttachmentStates[0];
				blendAttachmentState.blendEnable         = VK_TRUE;
				blendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
				blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
				blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
			}
			m_SceneMaterials[i]->PreparePipeline();
			m_SceneMaterials[i]->SetTexture("diffuseMap", m_SceneDiffuses[i]);
		}

		// collect meshles
		m_SceneMatMeshes.resize(m_SceneDiffuses.size());
		for (int32 i = 0; i < m_ModelScene->meshes.size(); ++i)
		{
			vk_demo::DVKMesh* mesh = m_ModelScene->meshes[i];
			const std::string& diffuseName = mesh->material.diffuse;
			if (diffuseName == "Anvil_Tex") {
				m_SceneMatMeshes[0].push_back(mesh);
			}
			else if (diffuseName == "BLACKSMITH_TEX") {
				m_SceneMatMeshes[1].push_back(mesh);
			}
			else if (diffuseName == "FloorTex") {
				m_SceneMatMeshes[2].push_back(mesh);
			}
			else if (diffuseName == "Spark_Diff") {
				mesh->linkNode->localMatrix.AppendScale(Vector3(2.5f, 2.5f, 2.5f));
				m_SceneMatMeshes[3].push_back(mesh);
			}
		}

		delete cmdBuffer;

		// Bright
		// 采样原始颜色，计算出非常亮的像素，并降级为做模糊准备。
		// RTColor -> RTQuater0
		m_BrightShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/25_Bloom/downsample.vert.spv",
			"assets/shaders/25_Bloom/downsample.frag.spv"
		);
		m_BrightMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RTTQuater0->GetRenderPass(),
			m_PipelineCache,
			m_BrightShader
		);
		m_BrightMaterial->PreparePipeline();
		m_BrightMaterial->SetTexture("diffuseTexture", m_RTColor);
		m_BrightMaterial->SetGlobalUniform("param", &m_FilterParam, sizeof(FilterParamBlock));

		// blurH
		// 使用降级后的RTQuater0进行水平模糊，然后存储到RTQuater1
		// RTQuater0 -> RTQuater1
		m_BlurHShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/25_Bloom/BlurH.vert.spv",
			"assets/shaders/25_Bloom/BlurH.frag.spv"
		);
		m_BlurHMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RTTQuater1->GetRenderPass(),
			m_PipelineCache,
			m_BlurHShader
		);
		m_BlurHMaterial->PreparePipeline();
		m_BlurHMaterial->SetTexture("diffuseTexture", m_RTColorQuater0);
		m_BlurHMaterial->SetGlobalUniform("param", &m_FilterParam, sizeof(FilterParamBlock));

		// blurV
		// 使用水平模糊后的RTQuater1进行垂直模糊，然后存储到RTQuater0
		// RTQuater1 -> RTQuater0
		m_BlurVShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/25_Bloom/BlurV.vert.spv",
			"assets/shaders/25_Bloom/BlurV.frag.spv"
		);
		m_BlurVMateria = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RTTQuater0->GetRenderPass(),
			m_PipelineCache,
			m_BlurVShader
		);
		m_BlurVMateria->PreparePipeline();
		m_BlurVMateria->SetTexture("diffuseTexture", m_RTColorQuater1);
		m_BlurVMateria->SetGlobalUniform("param", &m_FilterParam, sizeof(FilterParamBlock));

		// combine
		// 将模糊后的RTQuater0与RTColor进行合并
		m_CombineShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/25_Bloom/combine.vert.spv",
			"assets/shaders/25_Bloom/combine.frag.spv"
		);
		m_CombineMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_CombineShader
		);
		m_CombineMaterial->PreparePipeline();
		m_CombineMaterial->SetTexture("originTexture", m_RTColor);
		m_CombineMaterial->SetTexture("filterTexture", m_RTColorQuater0);
	}

	void DestroyAssets()
	{
		delete m_SceneShader;

		delete m_ModelScene;

		for (int32 i = 0; i < m_SceneDiffuses.size(); ++i) {
			delete m_SceneDiffuses[i];
		}
		m_SceneDiffuses.clear();

		for (int32 i = 0; i < m_SceneMaterials.size(); ++i) {
			delete m_SceneMaterials[i];
		}
		m_SceneMaterials.clear();

		delete m_CombineMaterial;
		delete m_CombineShader;

		delete m_BrightMaterial;
		delete m_BrightShader;

		delete m_BlurHMaterial;
		delete m_BlurHShader;
		delete m_BlurVMateria;
		delete m_BlurVShader;

		delete m_RTColor;
		delete m_RTDepth;

		delete m_RTColorQuater0;
		delete m_RTColorQuater1;
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
			m_RTTNormal->BeginRenderPass(commandBuffer);

			for (int32 i = 0; i < m_SceneMatMeshes.size(); ++i)
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SceneMaterials[i]->GetPipeline());
				for (int32 j = 0; j < m_SceneMatMeshes[i].size(); ++j) {
					m_SceneMaterials[i]->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
					m_SceneMatMeshes[i][j]->BindDrawCmd(commandBuffer);
				}
			}

			m_RTTNormal->EndRenderPass(commandBuffer);
		}

		// luminance
		{
			m_RTTQuater0->BeginRenderPass(commandBuffer);
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_BrightMaterial->GetPipeline());
				m_BrightMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Quad->meshes[0]->BindDrawCmd(commandBuffer);
			}
			m_RTTQuater0->EndRenderPass(commandBuffer);
		}

		// blurH
		{
			m_RTTQuater1->BeginRenderPass(commandBuffer);

			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_BlurHMaterial->GetPipeline());
				m_BlurHMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Quad->meshes[0]->BindDrawCmd(commandBuffer);
			}

			m_RTTQuater1->EndRenderPass(commandBuffer);
		}

		// blurV
		{
			m_RTTQuater0->BeginRenderPass(commandBuffer);
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_BlurVMateria->GetPipeline());
				m_BlurVMateria->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Quad->meshes[0]->BindDrawCmd(commandBuffer);
			}
			m_RTTQuater0->EndRenderPass(commandBuffer);
		}

		// combine pass
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

			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CombineMaterial->GetPipeline());
				m_CombineMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Quad->meshes[0]->BindDrawCmd(commandBuffer);
			}

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 0.10f, 3000.0f);
		m_ViewCamera.SetPosition(0, 150.0f, -250.0f);
		m_ViewCamera.LookAt(0, 0, 0);
        
        m_FilterParam.width   = m_FrameWidth / 4.0f;
        m_FilterParam.height  = m_FrameHeight / 4.0f;
        m_FilterParam.step    = 1.25f;
        m_FilterParam.bright  = 0.75f;
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

	vk_demo::DVKModel*			m_Quad = nullptr;

	vk_demo::DVKRenderTarget*   m_RTTNormal = nullptr;
	vk_demo::DVKRenderTarget*	m_RTTQuater0 = nullptr;
	vk_demo::DVKRenderTarget*	m_RTTQuater1 = nullptr;

	vk_demo::DVKTexture*        m_RTColor = nullptr;
	vk_demo::DVKTexture*        m_RTDepth = nullptr;
	vk_demo::DVKTexture*		m_RTColorQuater0 = nullptr;
	vk_demo::DVKTexture*		m_RTColorQuater1 = nullptr;

	ModelViewProjectionBlock	m_MVPData;

	vk_demo::DVKModel*			m_ModelScene = nullptr;
	vk_demo::DVKShader*			m_SceneShader = nullptr;
	TextureArray				m_SceneDiffuses;
	MaterialArray				m_SceneMaterials;
	MatMeshArray				m_SceneMatMeshes;

	FilterParamBlock            m_FilterParam;

	vk_demo::DVKMaterial*	    m_CombineMaterial;
	vk_demo::DVKShader*		    m_CombineShader;

	vk_demo::DVKMaterial*		m_BrightMaterial = nullptr;
	vk_demo::DVKShader*			m_BrightShader = nullptr;

	vk_demo::DVKMaterial*		m_BlurHMaterial = nullptr;
	vk_demo::DVKShader*			m_BlurHShader = nullptr;

	vk_demo::DVKMaterial*		m_BlurVMateria = nullptr;
	vk_demo::DVKShader*			m_BlurVShader = nullptr;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<BloomDemo>(1400, 900, "BloomDemo", cmdLine);
}
