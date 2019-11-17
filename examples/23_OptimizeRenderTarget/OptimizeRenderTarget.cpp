#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class OptimizeRenderTargetDemo : public DemoBase
{
public:
	OptimizeRenderTargetDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}

	virtual ~OptimizeRenderTargetDemo()
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
    
	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("OptimizeRenderTargetDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

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
        
		m_RTDepth = vk_demo::DVKTexture::CreateRenderTarget(
            m_VulkanDevice,
            PixelFormatToVkFormat(m_DepthFormat, false),
            VK_IMAGE_ASPECT_DEPTH_BIT,
            m_FrameWidth, m_FrameHeight,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        );
        
        vk_demo::DVKRenderPassInfo passInfo(
            m_RTColor, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
            m_RTDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
        );
        m_RenderTarget = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, passInfo);
	}
    
	void DestroyRenderTarget()
	{
        delete m_RenderTarget;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Quad = vk_demo::DVKDefaultRes::fullQuad;
		
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
			"assets/shaders/22_RenderTarget/obj.vert.spv",
			"assets/shaders/22_RenderTarget/obj.frag.spv"
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

        // filter
        m_FilterShader = vk_demo::DVKShader::Create(
				m_VulkanDevice,
				true,
				"assets/shaders/22_RenderTarget/FilterCGAColorspace.vert.spv",
				"assets/shaders/22_RenderTarget/FilterCGAColorspace.frag.spv"
			);
        m_FilterMaterial = vk_demo::DVKMaterial::Create(
            m_VulkanDevice,
            m_RenderPass,
            m_PipelineCache,
            m_FilterShader
        );
        m_FilterMaterial->PreparePipeline();
        m_FilterMaterial->SetTexture("inputImageTexture", m_RTColor);
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

        delete m_FilterMaterial;
        delete m_FilterShader;

		delete m_RTColor;
		delete m_RTDepth;
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
            m_RenderTarget->BeginRenderPass(commandBuffer);

			for (int32 i = 0; i < m_SceneMatMeshes.size(); ++i)
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SceneMaterials[i]->GetPipeline());
				for (int32 j = 0; j < m_SceneMatMeshes[i].size(); ++j) {
					m_SceneMaterials[i]->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
					m_SceneMatMeshes[i][j]->BindDrawCmd(commandBuffer);
				}
			}

            m_RenderTarget->EndRenderPass(commandBuffer);
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
            
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_FilterMaterial->GetPipeline());
				m_FilterMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Quad->meshes[0]->BindDrawCmd(commandBuffer);
			}
			
			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 10.0f, 5000.0f);
		m_ViewCamera.SetPosition(0, 500.0f, -1500.0f);
		m_ViewCamera.LookAt(0, 0, 0);
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
    vk_demo::DVKRenderTarget*   m_RenderTarget = nullptr;
    vk_demo::DVKTexture*        m_RTColor = nullptr;
    vk_demo::DVKTexture*        m_RTDepth = nullptr;
    
	ModelViewProjectionBlock	m_MVPData;
	vk_demo::DVKModel*			m_ModelScene = nullptr;
	vk_demo::DVKShader*			m_SceneShader = nullptr;
	TextureArray				m_SceneDiffuses;
	MaterialArray				m_SceneMaterials;
	MatMeshArray				m_SceneMatMeshes;
    
    vk_demo::DVKMaterial*	    m_FilterMaterial;
    vk_demo::DVKShader*		    m_FilterShader;
    
	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<OptimizeRenderTargetDemo>(1400, 900, "OptimizeRenderTargetDemo", cmdLine);
}
