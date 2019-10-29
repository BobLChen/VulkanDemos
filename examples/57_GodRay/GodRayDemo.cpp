#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class GodRayDemo : public DemoBase
{
public:
	GodRayDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~GodRayDemo()
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
		CreateSourceRT();
		CreateGodRayRT();
		CreateSunRT();
		LoadModelAssets();
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
			ImGui::Begin("GodRayDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void CreateSunRT()
	{
		m_TexSun = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			m_TexSourceColor->format, 
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_TexSourceColor->width, m_TexSourceColor->height,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKRenderPassInfo rttInfo(
			m_TexSun, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, nullptr
		);
		m_RTSun = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttInfo);
	}

	void CreateGodRayRT()
	{
		m_TexGodRay = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			m_TexSourceColor->format, 
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_TexSourceColor->width, m_TexSourceColor->height,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKRenderPassInfo rttInfo(
			m_TexGodRay, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			m_TexSourceDepth, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE
		);
		m_RTGodRay = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttInfo);
	}

	void CreateSourceRT()
	{
		m_TexSourceColor = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			VK_FORMAT_R8G8B8A8_UNORM, 
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		m_TexSourceDepth = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKRenderPassInfo rttInfo(
			m_TexSourceColor, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			m_TexSourceDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
		);
		m_RTSource = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttInfo);
	}

	void LoadModelAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// object
		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/halloween-pumpkin/model.fbx",
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
			"assets/models/halloween-pumpkin/BaseColor.jpg",
			m_VulkanDevice,
			cmdBuffer
		);

		m_TexNormal = vk_demo::DVKTexture::Create2D(
			"assets/models/halloween-pumpkin/Normal.jpg",
			m_VulkanDevice,
			cmdBuffer
		);

		m_Shader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/57_GodRay/obj.vert.spv",
			"assets/shaders/57_GodRay/obj.frag.spv"
		);

		m_Material = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RTSource->GetRenderPass(),
			m_PipelineCache,
			m_Shader
		);
		m_Material->PreparePipeline();
		m_Material->SetTexture("texAlbedo", m_TexAlbedo);
		m_Material->SetTexture("texNormal", m_TexNormal);

		// emissive
		m_EmissiveShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/57_GodRay/emissive.vert.spv",
			"assets/shaders/57_GodRay/emissive.frag.spv"
		);

		m_EmissiveMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RTSun->GetRenderPass(),
			m_PipelineCache,
			m_EmissiveShader
		);
		m_EmissiveMaterial->PreparePipeline();

		// god ray
		m_GodRayShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/57_GodRay/godray.vert.spv",
			"assets/shaders/57_GodRay/godray.frag.spv"
		);
		
		m_GodRayMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RTGodRay->GetRenderPass(),
			m_PipelineCache,
			m_GodRayShader
		);
		m_GodRayMaterial->PreparePipeline();

		// combine
		m_CombineShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/57_GodRay/combine.vert.spv",
			"assets/shaders/57_GodRay/combine.frag.spv"
		);

		m_CombineMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_CombineShader
		);
		m_CombineMaterial->PreparePipeline();
		m_CombineMaterial->SetTexture("originTexture",    m_TexSourceColor);
		m_CombineMaterial->SetTexture("volumeTexture",    m_TexSun);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;

		delete m_Shader;
		delete m_Material;

		delete m_TexAlbedo;
		delete m_TexNormal;

		delete m_TexSourceColor;
		delete m_TexSourceDepth;
		delete m_RTSource;

		delete m_TexGodRay;
		delete m_RTGodRay;

		delete m_CombineShader;
		delete m_CombineMaterial;

		delete m_GodRayShader;
		delete m_GodRayMaterial;

		delete m_EmissiveShader;
		delete m_EmissiveMaterial;

		delete m_RTSun;
		delete m_TexSun;
	}

	void DrawScene(VkCommandBuffer commandBuffer)
	{
		m_RTSource->BeginRenderPass(commandBuffer);

		for (int32 i = 0; i < m_Model->meshes.size(); ++i)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());

			m_Material->BeginFrame();

			m_MVPParam.model = m_Model->meshes[i]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = m_ViewCamera.GetView();
			m_MVPParam.proj  = m_ViewCamera.GetProjection();

			m_Material->BeginObject();
			m_Material->SetLocalUniform("uboMVP",   &m_MVPParam, sizeof(ModelViewProjectionBlock));
			m_Material->EndObject();

			m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_Model->meshes[i]->BindDrawCmd(commandBuffer);

			m_Material->EndFrame();
		}

		m_RTSource->EndRenderPass(commandBuffer);
	}

	void DrawFinal(VkCommandBuffer commandBuffer, int32 backBufferIndex)
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

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CombineMaterial->GetPipeline());
		m_CombineMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		vk_demo::DVKDefaultRes::fullQuad->meshes[0]->BindDrawCmd(commandBuffer);

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);
	}

	void DrawSun(VkCommandBuffer commandBuffer)
	{
		m_RTSun->BeginRenderPass(commandBuffer);

		m_MVPParam.model.SetIdentity();
		m_MVPParam.view = m_ViewCamera.GetView();
		m_MVPParam.proj = m_ViewCamera.GetProjection();

		m_EmissiveMaterial->BeginFrame();
		m_EmissiveMaterial->BeginObject();
		m_EmissiveMaterial->SetLocalUniform("uboMVP", &m_MVPParam, sizeof(ModelViewProjectionBlock));
		m_EmissiveMaterial->EndObject();
		m_EmissiveMaterial->EndFrame();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_EmissiveMaterial->GetPipeline());
		m_EmissiveMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		vk_demo::DVKDefaultRes::fullQuad->meshes[0]->BindDrawCmd(commandBuffer);

		m_RTSun->EndRenderPass(commandBuffer);
	}

	void DrawGodRay(VkCommandBuffer commandBuffer)
	{
		m_RTGodRay->BeginRenderPass(commandBuffer);

		

		m_RTGodRay->EndRenderPass(commandBuffer);
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		DrawScene(commandBuffer);
		DrawSun(commandBuffer);
		DrawGodRay(commandBuffer);
		DrawFinal(commandBuffer, backBufferIndex);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		m_ViewCamera.SetPosition(boundCenter.x, boundCenter.y, boundCenter.z - 5.0f);
		m_ViewCamera.LookAt(boundCenter);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 0.10f, 3000.0f);
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

	// object
	vk_demo::DVKModel*			m_Model = nullptr;
	vk_demo::DVKShader*			m_Shader = nullptr;
	vk_demo::DVKMaterial*		m_Material = nullptr;
	vk_demo::DVKTexture*		m_TexAlbedo = nullptr;
	vk_demo::DVKTexture*		m_TexNormal = nullptr;

	// emissive
	vk_demo::DVKShader*			m_EmissiveShader = nullptr;
	vk_demo::DVKMaterial*		m_EmissiveMaterial = nullptr;

	// combine
	vk_demo::DVKShader*			m_CombineShader = nullptr;
	vk_demo::DVKMaterial*		m_CombineMaterial = nullptr;

	// god ray
	vk_demo::DVKShader*			m_GodRayShader = nullptr;
	vk_demo::DVKMaterial*		m_GodRayMaterial = nullptr;

	// source
	vk_demo::DVKTexture*		m_TexSourceColor = nullptr;
	vk_demo::DVKTexture*		m_TexSourceDepth = nullptr;
	vk_demo::DVKRenderTarget*	m_RTSource = nullptr;

	// light source
	vk_demo::DVKTexture*		m_TexSun = nullptr;
	vk_demo::DVKRenderTarget*	m_RTSun = nullptr;

	// god ray
	vk_demo::DVKTexture*		m_TexGodRay = nullptr;
	vk_demo::DVKRenderTarget*	m_RTGodRay = nullptr;

	vk_demo::DVKCamera		    m_ViewCamera;

	ModelViewProjectionBlock	m_MVPParam;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<GodRayDemo>(1400, 900, "GodRayDemo", cmdLine);
}
