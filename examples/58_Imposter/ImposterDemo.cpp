#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class ImposterDemo : public DemoBase
{
public:
	ImposterDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~ImposterDemo()
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
		GenImposterAssets();
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
			ImGui::Begin("ImposterDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void GenImposterAssets()
	{
		int32 imageSize = 4096;

		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		vk_demo::DVKTexture* texSourceDepth = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			imageSize, imageSize,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		// diffuse rtt
		m_ImposterDiffuse = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			VK_FORMAT_R8G8B8A8_UNORM, 
			VK_IMAGE_ASPECT_COLOR_BIT,
			imageSize, imageSize,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKRenderPassInfo diffuseRttInfo(
			m_ImposterDiffuse, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			texSourceDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
		);
		vk_demo::DVKRenderTarget* diffuseRtSource = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, diffuseRttInfo);

		// normal rtt
		m_ImposterNormal = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			VK_FORMAT_R8G8B8A8_UNORM, 
			VK_IMAGE_ASPECT_COLOR_BIT,
			imageSize, imageSize,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKRenderPassInfo normalRttInfo(
			m_ImposterNormal, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			texSourceDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
		);
		vk_demo::DVKRenderTarget* normalRtSource = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, normalRttInfo);
		
		// imposter object
		vk_demo::DVKModel* model = vk_demo::DVKModel::LoadFromFile(
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
		model->rootNode->localMatrix.AppendRotation(180, Vector3::UpVector);

		vk_demo::DVKTexture* texAlbedo = vk_demo::DVKTexture::Create2D(
			"assets/models/halloween-pumpkin/BaseColor.jpg",
			m_VulkanDevice,
			cmdBuffer
		);

		vk_demo::DVKTexture* texNormal = vk_demo::DVKTexture::Create2D(
			"assets/models/halloween-pumpkin/Normal.jpg",
			m_VulkanDevice,
			cmdBuffer
		);

		// diffuse shader
		vk_demo::DVKShader* diffuseShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/58_Imposter/diffuse.vert.spv",
			"assets/shaders/58_Imposter/diffuse.frag.spv"
		);

		vk_demo::DVKMaterial* diffuseMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			diffuseRtSource->GetRenderPass(),
			m_PipelineCache,
			diffuseShader
		);
		diffuseMaterial->PreparePipeline();
		diffuseMaterial->SetTexture("texAlbedo", texAlbedo);
		diffuseMaterial->SetTexture("texNormal", texNormal);

		// normal shader
		vk_demo::DVKShader* normalShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/58_Imposter/normal.vert.spv",
			"assets/shaders/58_Imposter/normal.frag.spv"
		);

		vk_demo::DVKMaterial* normalMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			normalRtSource->GetRenderPass(),
			m_PipelineCache,
			normalShader
		);
		normalMaterial->PreparePipeline();
		normalMaterial->SetTexture("texAlbedo", texAlbedo);
		normalMaterial->SetTexture("texNormal", texNormal);

		// prepare
		vk_demo::DVKBoundingBox bounds = model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;

		Vector3 lookAt = boundCenter;
		float distance = 500.0f;
		float projSize = boundSize.Size() * 0.5f;

		vk_demo::DVKCamera imposterCamera;
		imposterCamera.Orthographic(-projSize, projSize, -projSize, projSize, 0.1, 3000);
		imposterCamera.SetPosition(Vector3(0, 0, -distance));

		float maxTileAngle = 180.0f;
		int32 tileSize     = imageSize / m_TileCount;

		VkViewport viewport = {};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = tileSize;
		scissor.extent.height = tileSize;

		const float DEGREES_TO_RADIANS = PI / 180.0f;

		// imposter diffuse
		{
			cmdBuffer->Begin();
			diffuseRtSource->BeginRenderPass(cmdBuffer->cmdBuffer);

			for (int32 i = 0; i < m_TileCount; ++i)
			{
				float tiltAngle = 90 - i * (maxTileAngle / (m_TileCount - 1)) - 180 / (m_TileCount * 2);

				for (int32 j = 0; j < m_TileCount; ++j)
				{
					float panAngle = 0 - j * (360 / m_TileCount) - 360 / (m_TileCount * 2);

					Vector3 target;
					target.x = lookAt.x + distance * MMath::Sin(panAngle  * DEGREES_TO_RADIANS) * MMath::Cos(tiltAngle * DEGREES_TO_RADIANS);
					target.z = lookAt.z + distance * MMath::Cos(panAngle  * DEGREES_TO_RADIANS) * MMath::Cos(tiltAngle * DEGREES_TO_RADIANS);
					target.y = lookAt.y + distance * MMath::Sin(tiltAngle * DEGREES_TO_RADIANS);

					imposterCamera.SetPosition(target);
					imposterCamera.LookAt(lookAt);

					{
						float tx = j * tileSize;
						float ty = i * tileSize;

						viewport.x       = tx;
						viewport.y       = tileSize + ty;
						viewport.width   = tileSize;
						viewport.height  = tileSize * -1;
						scissor.offset.x = tx;
						scissor.offset.y = ty;

						vkCmdSetViewport(cmdBuffer->cmdBuffer, 0, 1, &viewport);
						vkCmdSetScissor(cmdBuffer->cmdBuffer,  0, 1, &scissor);

						vkCmdBindPipeline(cmdBuffer->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, diffuseMaterial->GetPipeline());

						diffuseMaterial->BeginFrame();

						m_MVPParam.model = model->meshes[0]->linkNode->GetGlobalMatrix();
						m_MVPParam.view  = imposterCamera.GetView();
						m_MVPParam.proj  = imposterCamera.GetProjection();

						diffuseMaterial->BeginObject();
						diffuseMaterial->SetLocalUniform("uboMVP",   &m_MVPParam, sizeof(ModelViewProjectionBlock));
						diffuseMaterial->EndObject();

						diffuseMaterial->BindDescriptorSets(cmdBuffer->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
						model->meshes[0]->BindDrawCmd(cmdBuffer->cmdBuffer);

						diffuseMaterial->EndFrame();
					}
				}
			}

			diffuseRtSource->EndRenderPass(cmdBuffer->cmdBuffer);
			cmdBuffer->Submit();
		}

		// normal
		{
			cmdBuffer->Begin();
			normalRtSource->BeginRenderPass(cmdBuffer->cmdBuffer);

			for (int32 i = 0; i < m_TileCount; ++i)
			{
				float tiltAngle = 90 - i * (maxTileAngle / (m_TileCount - 1)) - 180 / (m_TileCount * 2);

				for (int32 j = 0; j < m_TileCount; ++j)
				{
					float panAngle = 0 - j * (360 / m_TileCount) - 360 / (m_TileCount * 2);

					Vector3 target;
					target.x = lookAt.x + distance * MMath::Sin(panAngle  * DEGREES_TO_RADIANS) * MMath::Cos(tiltAngle * DEGREES_TO_RADIANS);
					target.z = lookAt.z + distance * MMath::Cos(panAngle  * DEGREES_TO_RADIANS) * MMath::Cos(tiltAngle * DEGREES_TO_RADIANS);
					target.y = lookAt.y + distance * MMath::Sin(tiltAngle * DEGREES_TO_RADIANS);

					imposterCamera.SetPosition(target);
					imposterCamera.LookAt(lookAt);

					{
						float tx = j * tileSize;
						float ty = i * tileSize;

						viewport.x       = tx;
						viewport.y       = tileSize + ty;
						viewport.width   = tileSize;
						viewport.height  = tileSize * -1;
						scissor.offset.x = tx;
						scissor.offset.y = ty;

						vkCmdSetViewport(cmdBuffer->cmdBuffer, 0, 1, &viewport);
						vkCmdSetScissor(cmdBuffer->cmdBuffer,  0, 1, &scissor);

						vkCmdBindPipeline(cmdBuffer->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, normalMaterial->GetPipeline());

						normalMaterial->BeginFrame();

						m_MVPParam.model = model->meshes[0]->linkNode->GetGlobalMatrix();
						m_MVPParam.view  = imposterCamera.GetView();
						m_MVPParam.proj  = imposterCamera.GetProjection();

						normalMaterial->BeginObject();
						normalMaterial->SetLocalUniform("uboMVP",   &m_MVPParam, sizeof(ModelViewProjectionBlock));
						normalMaterial->EndObject();

						normalMaterial->BindDescriptorSets(cmdBuffer->cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
						model->meshes[0]->BindDrawCmd(cmdBuffer->cmdBuffer);

						normalMaterial->EndFrame();
					}
				}
			}

			normalRtSource->EndRenderPass(cmdBuffer->cmdBuffer);
			cmdBuffer->Submit();
		}

		delete model;

		delete texNormal;
		delete texAlbedo;

		delete texSourceDepth;

		delete diffuseShader;
		delete diffuseMaterial;
		delete diffuseRtSource;

		delete normalShader;
		delete normalMaterial;
		delete normalRtSource;

		delete cmdBuffer;
	}

	void LoadModelAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_ImposterShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/58_Imposter/imposter.vert.spv",
			"assets/shaders/58_Imposter/imposter.frag.spv"
		);

		m_ImposterMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_ImposterShader
		);
		m_ImposterMaterial->pipelineInfo.blendAttachmentStates[0].blendEnable         = VK_TRUE;
		m_ImposterMaterial->pipelineInfo.blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_ImposterMaterial->pipelineInfo.blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		m_ImposterMaterial->pipelineInfo.blendAttachmentStates[0].colorBlendOp        = VK_BLEND_OP_ADD;
		m_ImposterMaterial->pipelineInfo.blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_ImposterMaterial->pipelineInfo.blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		m_ImposterMaterial->pipelineInfo.blendAttachmentStates[0].alphaBlendOp        = VK_BLEND_OP_ADD;

		m_ImposterMaterial->PreparePipeline();
		m_ImposterMaterial->SetTexture("originTexture", m_ImposterDiffuse);
		m_ImposterMaterial->SetTexture("originNormal",  m_ImposterNormal);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_ImposterShader;
		delete m_ImposterMaterial;

		delete m_ImposterDiffuse;
		delete m_ImposterNormal;
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

		m_MVPParam.model.SetIdentity();
		// billboard
		m_MVPParam.model.LookAt(m_ViewCamera.GetTransform().GetOrigin());
		// flip quad face
		m_MVPParam.model.RotateY(180);
		// view projection
		m_MVPParam.view = m_ViewCamera.GetView();
		m_MVPParam.proj = m_ViewCamera.GetProjection();

		// calc uv from viewDir
		Vector3 viewDir = m_ViewCamera.GetTransform().GetForward();
		float xzX = viewDir.x;
		float xzZ = viewDir.z;
		float xzL = MMath::Sqrt(xzX * xzX + xzZ * xzZ);
		if (xzL != 0) {
			xzX = xzX / xzL;
			xzZ = xzZ / xzL;
		}

		float rotX = MMath::Acos(-xzZ) * 0.5;
		if (xzX >= 0) {
			rotX = PI - rotX;
		}
		rotX = 1.0 - rotX / PI;

		if (viewDir.y <= -1) {
			viewDir.y = -1;
		} else if (viewDir.y >= 1) {
			viewDir.y = 1;
		}
		float rotY = 1.0 - MMath::Acos(viewDir.y) / PI;

		float uScale = 1.0f / m_TileCount;
		float vScale = 1.0f / m_TileCount;
		float uStep  = int32(rotX * m_TileCount) * uScale;
		float vStep  = int32(rotY * m_TileCount) * vScale;

		Vector4 uvScale  = Vector4(uScale, vScale, uStep, vStep);
		Vector4 lightDir = -viewDir;

		m_ImposterMaterial->BeginFrame();
		m_ImposterMaterial->BeginObject();
		m_ImposterMaterial->SetLocalUniform("uboMVP",     &m_MVPParam, sizeof(ModelViewProjectionBlock));
		m_ImposterMaterial->SetLocalUniform("uboUVScale", &uvScale,    sizeof(Vector4));
		m_ImposterMaterial->SetLocalUniform("uboLight",   &lightDir,   sizeof(Vector4));
		m_ImposterMaterial->EndObject();
		m_ImposterMaterial->EndFrame();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ImposterMaterial->GetPipeline());
		m_ImposterMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		vk_demo::DVKDefaultRes::fullQuad->meshes[0]->BindDrawCmd(commandBuffer);

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.SetPosition(0, 0, -5.0f);
		m_ViewCamera.LookAt(0, 0, 0);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 1.0f, 100.0f);
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

	// combine
	vk_demo::DVKShader*			m_ImposterShader = nullptr;
	vk_demo::DVKMaterial*		m_ImposterMaterial = nullptr;

	// imposter diffuse + normal
	vk_demo::DVKTexture*		m_ImposterDiffuse = nullptr;
	vk_demo::DVKTexture*		m_ImposterNormal = nullptr;

	int32						m_TileCount = 16;

	vk_demo::DVKCamera		    m_ViewCamera;
	ModelViewProjectionBlock	m_MVPParam;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<ImposterDemo>(1400, 900, "ImposterDemo", cmdLine);
}
