#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

#define TEX_SIZE 4

class SSAODemo : public DemoBase
{
public:
	SSAODemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~SSAODemo()
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
		CreateSourceRT();
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

	struct DebugParamBlock
	{
		Vector4 data;
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
			ImGui::Begin("SSAODemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			// debug
			int mode = m_DebugParam.data.x;
			ImGui::SliderInt("Model", &mode, 0, 2);
			m_DebugParam.data.x = mode;

			ImGui::Separator();

			// params
			ImGui::SliderFloat("Blur Tolerance",			&m_BlurTolerance,		   -8.0f,  -1.0f);
			ImGui::SliderFloat("Upsample Tolerance",		&m_UpsampleTolerance,	   -12.0f, -1.0f);
			ImGui::SliderFloat("Noise Filter Threshold",	&m_NoiseFilterTolerance,   -8.0f,   0.0f);
			ImGui::SliderFloat("Screenspace Diameter",		&m_ScreenspaceDiameter,		5.0f,   15.0f);
			ImGui::SliderFloat("Rejection Falloff",			&m_RejectionFalloff,		1.0f,   10.0f);
			ImGui::SliderFloat("Accentuation",				&m_Accentuation,			0.0f,   1.0f);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void CreateSourceRT()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_TexSourceColor = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			cmdBuffer,
			m_SwapChain->GetColorFormat(),
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_TexSourceLinearDepth = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			cmdBuffer,
			VK_FORMAT_R32_SFLOAT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_TexSourceDepth = vk_demo::DVKTexture::CreateRenderTarget(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKTexture* rtColors[2];
		rtColors[0] = m_TexSourceColor;
		rtColors[1] = m_TexSourceLinearDepth;

		vk_demo::DVKRenderPassInfo rttInfo(
			2, rtColors, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
			m_TexSourceDepth, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE
		);
		m_RTSource = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, rttInfo);
		m_RTSource->colorLayout = ImageLayoutBarrier::ComputeGeneralRW;

		delete cmdBuffer;
	}

	void LoadSceneRes(vk_demo::DVKCommandBuffer* cmdBuffer)
	{
		m_SceneModel = vk_demo::DVKModel::LoadFromFile(
			"assets/models/Room/miniHouse_FBX.FBX",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position,
				VertexAttribute::VA_UV0,
				VertexAttribute::VA_Normal
			}
		);
		m_SceneModel->rootNode->localMatrix.AppendRotation(180, Vector3::UpVector);

		m_SceneShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/53_SSAO/obj.vert.spv",
			"assets/shaders/53_SSAO/obj.frag.spv"
		);

		const char* textures[TEX_SIZE] = {
			"assets/models/Room/miniHouse_Part1.jpg",
			"assets/models/Room/miniHouse_Part2.jpg",
			"assets/models/Room/miniHouse_Part3.jpg",
			"assets/models/Room/miniHouse_Part4.jpg"
		};

		for (int32 i = 0; i < TEX_SIZE; ++i)
		{
			m_SceneTextures[i] = vk_demo::DVKTexture::Create2D(
				textures[i],
				m_VulkanDevice,
				cmdBuffer
			);

			m_SceneMaterials[i] = vk_demo::DVKMaterial::Create(
				m_VulkanDevice,
				m_RTSource->GetRenderPass(),
				m_PipelineCache,
				m_SceneShader
			);
			m_SceneMaterials[i]->pipelineInfo.colorAttachmentCount = 2;
			m_SceneMaterials[i]->PreparePipeline();
			m_SceneMaterials[i]->SetTexture("diffuseMap", m_SceneTextures[i]);
		}
	}

	void LoadPreDepthRes(vk_demo::DVKCommandBuffer* cmdBuffer)
	{
		m_TexLinearDepth = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			cmdBuffer,
			VK_FORMAT_R32_SFLOAT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_TexSourceColor->width, m_TexSourceColor->height,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_TexDepthDownSize = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			cmdBuffer,
			VK_FORMAT_R32_SFLOAT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_TexSourceColor->width / 2, m_TexSourceColor->height / 2,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_TexDepthTiled = vk_demo::DVKTexture::Create2DArray(
			m_VulkanDevice,
			cmdBuffer,
			VK_FORMAT_R32_SFLOAT,
			VK_IMAGE_ASPECT_COLOR_BIT,
			Align(m_TexSourceColor->width, 8) / 8, Align(m_TexSourceColor->height, 8) / 8, 16,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_ShaderDepthPrepare = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/53_SSAO/DepthPrepare.comp.spv"
		);

		m_ComputeDepthPrepare = vk_demo::DVKCompute::Create(
			m_VulkanDevice, 
			m_PipelineCache, 
			m_ShaderDepthPrepare
		);
		m_ComputeDepthPrepare->SetStorageTexture("depthImage",  m_TexSourceLinearDepth);
		m_ComputeDepthPrepare->SetStorageTexture("linearImage", m_TexLinearDepth);
		m_ComputeDepthPrepare->SetStorageTexture("down2xImage", m_TexDepthDownSize);
		m_ComputeDepthPrepare->SetStorageTexture("down2xAtlas", m_TexDepthTiled);
	}

	void LoadComputeAoRes(vk_demo::DVKCommandBuffer* cmdBuffer)
	{
		m_TexAoMerge = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			cmdBuffer,
			VK_FORMAT_R8_UNORM,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_TexSourceColor->width / 2, m_TexSourceColor->height / 2,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_ShaderAoMerge = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/53_SSAO/ComputeAO.comp.spv"
		);

		m_ComputeAoMerge = vk_demo::DVKCompute::Create(
			m_VulkanDevice, 
			m_PipelineCache, 
			m_ShaderAoMerge
		);
		m_ComputeAoMerge->SetStorageTexture("depthImage", m_TexDepthTiled);
		m_ComputeAoMerge->SetStorageTexture("outAoImage", m_TexAoMerge);
	}

	void LoadCombineRes(vk_demo::DVKCommandBuffer* cmdBuffer)
	{
		m_CombineShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/53_SSAO/combine.vert.spv",
			"assets/shaders/53_SSAO/combine.frag.spv"
		);

		m_CombineMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_CombineShader
		);
		m_CombineMaterial->PreparePipeline();
		m_CombineMaterial->SetTexture("ssaoTexture",    m_TexAoFullScreen);
		m_CombineMaterial->SetTexture("originTexture",  m_TexSourceColor);
	}

	void LoadBlurAndUpsampleRes(vk_demo::DVKCommandBuffer* cmdBuffer)
	{
		m_TexAoFullScreen = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			cmdBuffer,
			VK_FORMAT_R8_UNORM,
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_TexSourceColor->width, m_TexSourceColor->height,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_ShaderBlurAndUpsample = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/53_SSAO/AoBlurUpsample.comp.spv"
		);

		m_ComputeBlurAndUpsample = vk_demo::DVKCompute::Create(
			m_VulkanDevice, 
			m_PipelineCache, 
			m_ShaderBlurAndUpsample
		);
		m_ComputeBlurAndUpsample->SetStorageTexture("texLowDepth",		m_TexDepthDownSize);
		m_ComputeBlurAndUpsample->SetStorageTexture("texHighDepth",		m_TexLinearDepth);
		m_ComputeBlurAndUpsample->SetStorageTexture("texintervalDepth",	m_TexAoMerge);
		m_ComputeBlurAndUpsample->SetStorageTexture("texOutAO",			m_TexAoFullScreen);
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Quad = vk_demo::DVKDefaultRes::fullQuad;

		LoadSceneRes(cmdBuffer);
		LoadPreDepthRes(cmdBuffer);
		LoadComputeAoRes(cmdBuffer);
		LoadBlurAndUpsampleRes(cmdBuffer);
		LoadCombineRes(cmdBuffer);
		
		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_SceneModel;
		delete m_SceneShader;

		for (int32 i = 0; i < TEX_SIZE; ++i)
		{
			delete m_SceneTextures[i];
			delete m_SceneMaterials[i];
		}

		// source
		{
			delete m_TexSourceColor;
			delete m_TexSourceLinearDepth;
			delete m_TexSourceDepth;
			delete m_RTSource;
		}

		// prepare detph
		{
			delete m_ShaderDepthPrepare;
			delete m_ComputeDepthPrepare;

			delete m_TexLinearDepth;
			delete m_TexDepthDownSize;
			delete m_TexDepthTiled;
		}

		// ao merge
		{
			delete m_ShaderAoMerge;
			delete m_ComputeAoMerge;
			delete m_TexAoMerge;
		}

		// blur and upsample 
		{
			delete m_ShaderBlurAndUpsample;
			delete m_ComputeBlurAndUpsample;
			delete m_TexAoFullScreen;
		}

		// combine
		{
			delete m_CombineShader;
			delete m_CombineMaterial;
		}
	}

	void ScenePass(VkCommandBuffer commandBuffer)
	{
		m_RTSource->BeginRenderPass(commandBuffer);

		vk_demo::DVKMaterial* materials[7] = {
			m_SceneMaterials[3],
			m_SceneMaterials[2],
			m_SceneMaterials[1],
			m_SceneMaterials[0],
			m_SceneMaterials[0],
			m_SceneMaterials[3],
			m_SceneMaterials[0],
		};

		for (int32 i = 0; i < m_SceneModel->meshes.size(); ++i)
		{
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materials[i]->GetPipeline());

			materials[i]->BeginFrame();

			m_MVPParam.model = m_SceneModel->meshes[i]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = m_ViewCamera.GetView();
			m_MVPParam.proj  = m_ViewCamera.GetProjection();

			materials[i]->BeginObject();
			materials[i]->SetLocalUniform("uboMVP", &m_MVPParam, sizeof(ModelViewProjectionBlock));
			materials[i]->EndObject();

			materials[i]->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_SceneModel->meshes[i]->BindDrawCmd(commandBuffer);

			materials[i]->EndFrame();
		}

		m_RTSource->EndRenderPass(commandBuffer);
	}

	void CombinePass(VkCommandBuffer commandBuffer, int32 backBufferIndex)
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

		// combine pass
		{
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

			m_CombineMaterial->BeginFrame();
			m_CombineMaterial->BeginObject();
			m_CombineMaterial->SetLocalUniform("paramData", &m_DebugParam, sizeof(DebugParamBlock));
			m_CombineMaterial->EndObject();

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_CombineMaterial->GetPipeline());
			m_CombineMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_Quad->meshes[0]->BindDrawCmd(commandBuffer);

			m_CombineMaterial->EndFrame();
		}

		// ui pass
		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		ScenePass(commandBuffer);
		PrepareDepthPass(commandBuffer);
		ComputeAoPass(commandBuffer);
		BlurAndUpsamplePass(commandBuffer);
		CombinePass(commandBuffer, backBufferIndex);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void BlurAndUpsamplePass(VkCommandBuffer commandBuffer)
	{
		int32 lowWidth   = m_TexDepthDownSize->width;
		int32 lowHeight  = m_TexDepthDownSize->height;
		int32 highWidth  = m_TexLinearDepth->width;
		int32 highHeight = m_TexLinearDepth->height;

		float blurTolerance     = MMath::Pow(1.0f - MMath::Pow(10.0f, m_BlurTolerance) * m_FrameWidth / lowWidth, 2);
		float upsampleTolerance = MMath::Pow(10.0f, m_UpsampleTolerance);
		float noiseFilterWeight = 1.0f / (MMath::Pow(10.0f, m_NoiseFilterTolerance) + upsampleTolerance);

		float paramData[8] = {
			1.0f / lowWidth, 
			1.0f / lowHeight, 
			1.0f / highWidth, 
			1.0f / highHeight, 
			noiseFilterWeight, 
			m_FrameWidth * 1.0f / lowWidth, 
			blurTolerance, 
			upsampleTolerance
		};

		int32 groupSizeX = Align((highWidth  + 2), 16) / 16;
		int32 groupSizeY = Align((highHeight + 2), 16) / 16;
		m_ComputeBlurAndUpsample->SetUniform("paramData", paramData, 8 * sizeof(float));
		m_ComputeBlurAndUpsample->BindDispatch(commandBuffer, groupSizeX, groupSizeY, 1);
	}

	void ComputeAoPass(VkCommandBuffer commandBuffer)
	{
		float tanHalfFov   = 1.0f / (1.0 / MMath::Tan(m_ViewCamera.GetFov() * 0.5f));

		int32 bufferWidth  = m_TexDepthTiled->width;
		int32 bufferHeight = m_TexDepthTiled->height;
		int32 arrayCount   = m_TexDepthTiled->layerCount;

		float thicknessMultiplier = 2.0f * tanHalfFov * m_ScreenspaceDiameter / bufferWidth;
		float inverseRangeFactor  = 1.0f / thicknessMultiplier;

		float paramDatas[28];
		paramDatas[0]  = inverseRangeFactor / m_SampleThickness[0];
		paramDatas[1]  = inverseRangeFactor / m_SampleThickness[1];
		paramDatas[2]  = inverseRangeFactor / m_SampleThickness[2];
		paramDatas[3]  = inverseRangeFactor / m_SampleThickness[3];
		paramDatas[4]  = inverseRangeFactor / m_SampleThickness[4];
		paramDatas[5]  = inverseRangeFactor / m_SampleThickness[5];
		paramDatas[6]  = inverseRangeFactor / m_SampleThickness[6];
		paramDatas[7]  = inverseRangeFactor / m_SampleThickness[7];
		paramDatas[8]  = inverseRangeFactor / m_SampleThickness[8];
		paramDatas[9]  = inverseRangeFactor / m_SampleThickness[9];
		paramDatas[10] = inverseRangeFactor / m_SampleThickness[10];
		paramDatas[11] = inverseRangeFactor / m_SampleThickness[11];

		paramDatas[12] = 4.0f * m_SampleThickness[0];     // Axial
		paramDatas[13] = 4.0f * m_SampleThickness[1];     // Axial
		paramDatas[14] = 4.0f * m_SampleThickness[2];     // Axial
		paramDatas[15] = 4.0f * m_SampleThickness[3];     // Axial
		paramDatas[16] = 4.0f * m_SampleThickness[4];     // Diagonal
		paramDatas[17] = 8.0f * m_SampleThickness[5];     // L-shaped
		paramDatas[18] = 8.0f * m_SampleThickness[6];     // L-shaped
		paramDatas[19] = 8.0f * m_SampleThickness[7];     // L-shaped
		paramDatas[20] = 4.0f * m_SampleThickness[8];     // Diagonal
		paramDatas[21] = 8.0f * m_SampleThickness[9];     // L-shaped
		paramDatas[22] = 8.0f * m_SampleThickness[10];    // L-shaped
		paramDatas[23] = 4.0f * m_SampleThickness[11];    // Diagonal

		paramDatas[12] = 0.0f;
		paramDatas[14] = 0.0f;
		paramDatas[17] = 0.0f;
		paramDatas[19] = 0.0f;
		paramDatas[21] = 0.0f;

		float totalWeight = 0.0f;
		for (int i = 12; i < 24; ++i) {
			totalWeight += paramDatas[i];
		}

		for (int i = 12; i < 24; ++i) {
			paramDatas[i] /= totalWeight;
		}

		paramDatas[24] = 1.0f / bufferWidth;
		paramDatas[25] = 1.0f / bufferHeight;
		paramDatas[26] = 1.0f / -m_RejectionFalloff;
		paramDatas[27] = 1.0f / (1.0f + m_Accentuation);

		int32 groupSizeX = Align(bufferWidth,  8) / 8;
		int32 groupSizeY = Align(bufferHeight, 8) / 8;
		m_ComputeAoMerge->SetUniform("paramData", paramDatas, 28 * sizeof(float));
		m_ComputeAoMerge->BindDispatch(commandBuffer, groupSizeX, groupSizeY, arrayCount);
	}

	void PrepareDepthPass(VkCommandBuffer commandBuffer)
	{
		float params[4] = { m_ViewCamera.GetFar() - m_ViewCamera.GetNear(), 0, 0, 0 };
		m_ComputeDepthPrepare->SetUniform("paramData", params, sizeof(float) * 4);

		int32 groupSizeX = Align(m_TexSourceColor->width,  16) / 16;
		int32 groupSizeY = Align(m_TexSourceColor->height, 16) / 16;
		m_ComputeDepthPrepare->BindDispatch(commandBuffer, groupSizeX, groupSizeY, 1);
	}

	void InitParmas()
	{
		m_ViewCamera.SetPosition(0, 500.0f, -1500.0f);
		m_ViewCamera.LookAt(0, 100.0f, 0);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 10.0f, 3000.0f);
		
		m_SampleThickness[0]  = MMath::Sqrt(1.0f - 0.2f * 0.2f);
		m_SampleThickness[1]  = MMath::Sqrt(1.0f - 0.4f * 0.4f);
		m_SampleThickness[2]  = MMath::Sqrt(1.0f - 0.6f * 0.6f);
		m_SampleThickness[3]  = MMath::Sqrt(1.0f - 0.8f * 0.8f);
		m_SampleThickness[4]  = MMath::Sqrt(1.0f - 0.2f * 0.2f - 0.2f * 0.2f);
		m_SampleThickness[5]  = MMath::Sqrt(1.0f - 0.2f * 0.2f - 0.4f * 0.4f);
		m_SampleThickness[6]  = MMath::Sqrt(1.0f - 0.2f * 0.2f - 0.6f * 0.6f);
		m_SampleThickness[7]  = MMath::Sqrt(1.0f - 0.2f * 0.2f - 0.8f * 0.8f);
		m_SampleThickness[8]  = MMath::Sqrt(1.0f - 0.4f * 0.4f - 0.4f * 0.4f);
		m_SampleThickness[9]  = MMath::Sqrt(1.0f - 0.4f * 0.4f - 0.6f * 0.6f);
		m_SampleThickness[10] = MMath::Sqrt(1.0f - 0.4f * 0.4f - 0.8f * 0.8f);
		m_SampleThickness[11] = MMath::Sqrt(1.0f - 0.6f * 0.6f - 0.6f * 0.6f);
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
	float						m_SampleThickness[12];

	vk_demo::DVKModel*			m_Quad = nullptr;

	// scene
	vk_demo::DVKModel*			m_SceneModel = nullptr;
	vk_demo::DVKShader*			m_SceneShader = nullptr;
	vk_demo::DVKTexture*		m_SceneTextures[TEX_SIZE];
	vk_demo::DVKMaterial*		m_SceneMaterials[TEX_SIZE];

	// source
	vk_demo::DVKTexture*		m_TexSourceColor = nullptr;
	vk_demo::DVKTexture*		m_TexSourceLinearDepth = nullptr;
	vk_demo::DVKTexture*		m_TexSourceDepth = nullptr;
	vk_demo::DVKRenderTarget*	m_RTSource = nullptr;

	// prepare depth
	vk_demo::DVKTexture*		m_TexLinearDepth = nullptr;
	vk_demo::DVKTexture*		m_TexDepthDownSize = nullptr;
	vk_demo::DVKTexture*		m_TexDepthTiled = nullptr;
	vk_demo::DVKShader*         m_ShaderDepthPrepare = nullptr;
	vk_demo::DVKCompute*   		m_ComputeDepthPrepare = nullptr;

	// ao merge
	vk_demo::DVKShader*         m_ShaderAoMerge = nullptr;
	vk_demo::DVKCompute*   		m_ComputeAoMerge = nullptr;
	vk_demo::DVKTexture*		m_TexAoMerge = nullptr;

	// upsample and blue
	vk_demo::DVKShader*         m_ShaderBlurAndUpsample = nullptr;
	vk_demo::DVKCompute*   		m_ComputeBlurAndUpsample = nullptr;
	vk_demo::DVKTexture*		m_TexAoFullScreen = nullptr;

	// finnal pass
	vk_demo::DVKShader*			m_CombineShader = nullptr;
	vk_demo::DVKMaterial*		m_CombineMaterial = nullptr;

	vk_demo::DVKCamera		    m_ViewCamera;

	ModelViewProjectionBlock	m_MVPParam;
	DebugParamBlock				m_DebugParam;

	float						m_BlurTolerance        = -5.0f;
	float						m_UpsampleTolerance    = -7.0f;
	float						m_NoiseFilterTolerance = -3.0f;
	float						m_ScreenspaceDiameter  = 10.0f;
	float						m_RejectionFalloff	   = 2.5f;
	float						m_Accentuation		   = 0.10f;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<SSAODemo>(1400, 900, "SSAODemo", cmdLine);
}
