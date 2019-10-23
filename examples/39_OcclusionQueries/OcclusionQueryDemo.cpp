#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"

#include <vector>

#define OBJECT_COUNT 1024

class OcclusionQueryDemo : public DemoBase
{
public:
	OcclusionQueryDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~OcclusionQueryDemo()
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

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);
		bool hovered = UpdateUI(time, delta);

		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		vkGetQueryPoolResults(
			m_Device, 
			m_QueryPool, 
			0, OBJECT_COUNT, sizeof(uint64) * OBJECT_COUNT, m_QuerySamples, sizeof(uint64), 
			VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT
		);

		SetupCommandBuffers(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("OcclusionQueryDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Checkbox("EnableQuery", &m_EnableQuery);

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

		VkQueryPoolCreateInfo queryPoolCreateInfo;
		ZeroVulkanStruct(queryPoolCreateInfo, VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO);
		queryPoolCreateInfo.queryType  = VK_QUERY_TYPE_OCCLUSION;
		queryPoolCreateInfo.queryCount = OBJECT_COUNT;
		VERIFYVULKANRESULT(vkCreateQueryPool(m_Device, &queryPoolCreateInfo, VULKAN_CPU_ALLOCATOR, &m_QueryPool));

		m_ModelSphere = vk_demo::DVKModel::LoadFromFile(
			"assets/models/sphere.obj",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position, 
				VertexAttribute::VA_Normal
			}
		);

		m_ModelGround = vk_demo::DVKModel::LoadFromFile(
			"assets/models/plane_circle.fbx",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position, 
				VertexAttribute::VA_Normal
			}
		);
		m_ModelGround->rootNode->localMatrix.AppendRotation(270.0f, Vector3::RightVector);
		m_ModelGround->rootNode->localMatrix.AppendScale(Vector3(500, 500, 500));

		vk_demo::DVKBoundingBox bounds = m_ModelSphere->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;
		m_SphereCenter = boundCenter;
		m_SphereRadius = boundSize.Size();

		m_ModelSphere->rootNode->localMatrix.AppendTranslation(Vector3(0, 19.73f, 0));

		for (int32 i = 0; i < OBJECT_COUNT; ++i)
		{
			m_ObjModels[i].AppendRotation(MMath::FRandRange(0.0f, 360.0f), Vector3::UpVector, nullptr);
			m_ObjModels[i].AppendTranslation(Vector3(
				MMath::FRandRange(-250.0f, 250.0f),
				19.73f,
				MMath::FRandRange(-250.0f, 250.0f)
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

		m_SimpleShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/39_OcclusionQueries/Simple.vert.spv",
			"assets/shaders/39_OcclusionQueries/Simple.frag.spv"
		);
		m_SimpleMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_SimpleShader
		);
		m_SimpleMaterial->PreparePipeline();
        
		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_ModelGround;
		delete m_ModelSphere;
        
		delete m_SimpleMaterial;
		delete m_SimpleShader;
		
		delete m_Material;
		delete m_Shader;

		vkDestroyQueryPool(m_Device, m_QueryPool, VULKAN_CPU_ALLOCATOR);
	}

	void RenderOcclusions(VkCommandBuffer commandBuffer, vk_demo::DVKCamera& camera)
	{
		m_SimpleMaterial->BeginFrame();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_SimpleMaterial->GetPipeline());
		// bind only
		m_ModelSphere->meshes[0]->BindOnly(commandBuffer);

		for (int32 i = 0; i < OBJECT_COUNT; ++i)
		{
			m_MVPParam.model = m_ObjModels[i];
			m_MVPParam.view  = camera.GetView();
			m_MVPParam.proj  = camera.GetProjection();

			m_SimpleMaterial->BeginObject();
			m_SimpleMaterial->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
			m_SimpleMaterial->EndObject();

			vkCmdBeginQuery(commandBuffer, m_QueryPool, i, VK_QUERY_CONTROL_PRECISE_BIT);

			m_SimpleMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, i);
			m_ModelSphere->meshes[0]->DrawOnly(commandBuffer);

			vkCmdEndQuery(commandBuffer, m_QueryPool, i);
		}

		m_SimpleMaterial->EndFrame();
	}

	void RenderGround(VkCommandBuffer commandBuffer, vk_demo::DVKCamera& camera)
	{
		m_Material->BeginFrame();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());
		
		m_MVPParam.model = m_ModelGround->rootNode->GetGlobalMatrix();
		m_MVPParam.view  = camera.GetView();
		m_MVPParam.proj  = camera.GetProjection();

		m_Material->BeginObject();
		m_Material->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
		m_Material->EndObject();

		m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		m_ModelGround->meshes[0]->BindDrawCmd(commandBuffer);

		m_Material->EndFrame();
	}

	void RenderSpheres(VkCommandBuffer commandBuffer, vk_demo::DVKCamera& camera)
	{
		m_Material->BeginFrame();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());
		m_ModelSphere->meshes[0]->BindOnly(commandBuffer);
		
		int32 count = 0;
		for (int32 i = 0; i < OBJECT_COUNT; ++i)
		{
			bool occluded = m_QuerySamples[i] <= 50; // precise: m_QuerySamples[i] == 0
			if (occluded && m_EnableQuery) {
				continue;
			}

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

		m_Material->EndFrame();
	}

	void BeginMainPass(VkCommandBuffer commandBuffer, int32 backBufferIndex)
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
	}

	void SetupCommandBuffers(int32 backBufferIndex)
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

		vkCmdResetQueryPool(commandBuffer, m_QueryPool, 0, OBJECT_COUNT);

		BeginMainPass(commandBuffer, backBufferIndex);

		// query pool
		{
			viewport.y = m_FrameHeight * 0.5f;
			scissor.offset.y = 0;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			RenderOcclusions(commandBuffer, m_ViewCamera);
		}

		// clear scene
		{
			VkClearAttachment clearAttachments[2] = {};
			clearAttachments[0].aspectMask       = VK_IMAGE_ASPECT_COLOR_BIT;
			clearAttachments[0].clearValue.color = { { 0.2f, 0.2f, 0.2f, 1.0f } };
			clearAttachments[0].colorAttachment  = 0;
			clearAttachments[1].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			clearAttachments[1].clearValue.depthStencil = { 1.0f, 0 };

			VkClearRect clearRect = {};
			clearRect.layerCount  = 1;
			clearRect.rect.offset = { 0, 0 };
			clearRect.rect.extent = scissor.extent;

			vkCmdClearAttachments(commandBuffer, 2, clearAttachments, 1, &clearRect);
		}

		// normal
		{
			viewport.y = m_FrameHeight * 0.5f;
			scissor.offset.y = 0;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			RenderSpheres(commandBuffer, m_ViewCamera);
			RenderGround(commandBuffer, m_ViewCamera);
		}
		
		// occlusion view
		{
			viewport.y = m_FrameHeight;
			scissor.offset.y = m_FrameHeight * 0.5f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			RenderSpheres(commandBuffer, m_TopCamera);
			RenderGround(commandBuffer, m_TopCamera);
		}
		
		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.freeze.x = 1.0f;
		m_ViewCamera.SetPosition(0, 19.73f, -800.0f);
		m_ViewCamera.LookAt(0, 19.73f, 0);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight() * 0.5f, 1.0f, 1500.0f);

		m_TopCamera.SetPosition(-500, 1500, 0);
		m_TopCamera.LookAt(0, 0, 0);
		m_TopCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight() * 0.5f, 1.0f, 3000.0f);

		memset(m_QuerySamples, 65535, sizeof(uint64) * OBJECT_COUNT);
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

	VkQueryPool					m_QueryPool;

	vk_demo::DVKModel*			m_ModelSphere = nullptr;
	vk_demo::DVKModel*			m_ModelGround = nullptr;
	vk_demo::DVKMaterial*		m_Material = nullptr;
	vk_demo::DVKShader*			m_Shader = nullptr;
	vk_demo::DVKMaterial*		m_SimpleMaterial = nullptr;
	vk_demo::DVKShader*			m_SimpleShader = nullptr;

	Matrix4x4					m_ObjModels[OBJECT_COUNT];
	uint64						m_QuerySamples[OBJECT_COUNT];
	Vector3						m_SphereCenter;
	float						m_SphereRadius;
	bool						m_EnableQuery = true;

	vk_demo::DVKCamera		    m_ViewCamera;
	vk_demo::DVKCamera			m_TopCamera;

	ModelViewProjectionBlock	m_MVPParam;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<OcclusionQueryDemo>(1400, 900, "OcclusionQueryDemo", cmdLine);
}
