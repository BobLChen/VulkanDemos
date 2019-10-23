#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Math/Quat.h"

#include <vector>

enum FXAATypes
{
	Normal = 0,
	Default,
	Fast,
	High,
	Best,
	Count,
};

class FXAADemo : public DemoBase
{
public:
	FXAADemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~FXAADemo()
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
        
		CreateRenderTarget();
        LoadAssets();
		CreateMaterials();
		InitParmas();
		CreateGUI();
        
		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
		DemoBase::Release();
		DestroyRenderTarget();
		DestroyMaterials();
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

	struct FXAAParamBlock
	{
		Vector2 frame;
		Vector2 padding;
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
        
		if (m_AutoRotate) {
			m_LineModel->rootNode->localMatrix.AppendRotation(delta * 15.0f, Vector3::UpVector);
		}

		// model
		m_LineMaterial->BeginFrame();
		m_MVPData.model = m_LineModel->meshes[0]->linkNode->GetGlobalMatrix();
		m_LineMaterial->BeginObject();
		m_LineMaterial->SetLocalUniform("uboMVP", &m_MVPData, sizeof(ModelViewProjectionBlock));
		m_LineMaterial->EndObject();
		m_LineMaterial->EndFrame();

		// filter
		vk_demo::DVKMaterial* material = m_FilterMaterials[m_Select];
		material->BeginFrame();
		material->BeginObject();
		material->SetLocalUniform("fxaaParam", &m_FXAAParam, sizeof(FXAAParamBlock));
		material->EndObject();
		material->EndFrame();
		
		SetupCommandBuffers(bufferIndex);
        
		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("FXAADemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            
			ImGui::Checkbox("Auto Spin", &m_AutoRotate);

			int select = m_Select;
			ImGui::Combo("Quality", &select, m_FilterNames.data(), m_FilterNames.size());
			m_Select = (FXAATypes)select;

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void GenerateLineSphere(std::vector<float>& outVertices, int32 sphslices, float scale)
	{
		int32 count  = 0;
		int32 slices = sphslices;
		int32 stacks = slices;

		outVertices.resize((slices + 1) * stacks * 3 * 2);

		float ds = 1.0f / sphslices;
		float dt = 1.0f / sphslices;
		float t  = 1.0;
		float drho   = PI / stacks;
		float dtheta = 2.0 * PI / slices;

		for (int32 i= 0; i < stacks; ++i) 
		{
			float rho = i * drho;
			float s   = 0.0;
			for (int32 j = 0; j<=slices; ++j) {
				float theta = (j == slices) ? 0.0f : j * dtheta;
				float x = -sin(theta) * sin(rho) * scale;
				float z =  cos(theta) * sin(rho) * scale;
				float y = -cos(rho) * scale;

				outVertices[count + 0] = x;
				outVertices[count + 1] = y;
				outVertices[count + 2] = z;
				count += 3;

				x = -sin(theta) * sin(rho+drho) * scale;
				z =  cos(theta) * sin(rho+drho) * scale;
				y = -cos(rho+drho) * scale;

				outVertices[count + 0] = x;
				outVertices[count + 1] = y;
				outVertices[count + 2] = z;
				count += 3;

				s += ds;
			}
			t -= dt;
		}
	}
    
	void DestroyMaterials()
	{
		delete m_LineShader;
		delete m_LineMaterial;

		delete m_NormalShader;
		delete m_NormalMaterial;

		delete m_FXAADefaultMaterial;
		delete m_FXAADefaultShader;

		delete m_FXAAFastShader;
		delete m_FXAAFastMaterial;

		delete m_FXAAHighShader;
		delete m_FXAAHighMaterial;

		delete m_FXAABestMaterial;
		delete m_FXAABestShader;
	}

	void CreateMaterials()
	{
		m_LineShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/32_FXAA/obj.vert.spv",
			"assets/shaders/32_FXAA/obj.frag.spv"
		);

		float range0 = m_VulkanDevice->GetLimits().lineWidthRange[0];
		float range1 = m_VulkanDevice->GetLimits().lineWidthRange[1];
		float lineWidth = MMath::Clamp(3.0f, range0, range1);
        lineWidth = MMath::Min(lineWidth, m_VulkanDevice->GetLimits().lineWidthRange[1]);

		m_LineMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_LineShader
		);
		m_LineMaterial->pipelineInfo.inputAssemblyState.topology    = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		m_LineMaterial->pipelineInfo.rasterizationState.cullMode    = VK_CULL_MODE_NONE;
		m_LineMaterial->pipelineInfo.rasterizationState.lineWidth   = lineWidth; // 注意这里特别重要，如果非常细的Line，FXAA是搞不定的。
		m_LineMaterial->pipelineInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		m_LineMaterial->PreparePipeline();

		// fxaa-default
		m_FXAADefaultShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/32_FXAA/FXAA.vert.spv",
			"assets/shaders/32_FXAA/FXAA_Default.frag.spv"
		);

		// fxaa-fast
		m_FXAAFastShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/32_FXAA/FXAA.vert.spv",
			"assets/shaders/32_FXAA/FXAA_Fastest.frag.spv"
		);

		// fxaa-hight
		m_FXAAHighShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/32_FXAA/FXAA.vert.spv",
			"assets/shaders/32_FXAA/FXAA_High_Quality.frag.spv"
		);

		// fxaa-best
		m_FXAABestShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/32_FXAA/FXAA.vert.spv",
			"assets/shaders/32_FXAA/FXAA_Extreme_Quality.frag.spv"
		);

		// faxaa-none
		m_NormalShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/32_FXAA/Normal.vert.spv",
			"assets/shaders/32_FXAA/Normal.frag.spv"
		);

		// fxaa-none
		m_NormalMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_NormalShader
		);
		m_NormalMaterial->PreparePipeline();
		m_NormalMaterial->SetTexture("sourceTexture", m_RTColor);

		// fxaa-default
		m_FXAADefaultMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FXAADefaultShader
		);
		m_FXAADefaultMaterial->PreparePipeline();
		m_FXAADefaultMaterial->SetTexture("sourceTexture", m_RTColor);

		// fxaa-fast
		m_FXAAFastMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FXAAFastShader
		);
		m_FXAAFastMaterial->PreparePipeline();
		m_FXAAFastMaterial->SetTexture("sourceTexture", m_RTColor);

		// fxaa-high
		m_FXAAHighMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FXAAHighShader
		);
		m_FXAAHighMaterial->PreparePipeline();
		m_FXAAHighMaterial->SetTexture("sourceTexture", m_RTColor);

		// fxaa-best
		m_FXAABestMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_FXAABestShader
		);
		m_FXAABestMaterial->PreparePipeline();
		m_FXAABestMaterial->SetTexture("sourceTexture", m_RTColor);
		
		m_FilterNames.resize(FXAATypes::Count);
		m_FilterNames[FXAATypes::Normal]	= "None";
		m_FilterNames[FXAATypes::Default]	= "FXAA-Default";
		m_FilterNames[FXAATypes::Fast]		= "FXAA-Fast";
		m_FilterNames[FXAATypes::High]		= "FXAA-High";
		m_FilterNames[FXAATypes::Best]		= "FXAA-Best";

		m_FilterMaterials.resize(FXAATypes::Count);
		m_FilterMaterials[FXAATypes::Normal]	= m_NormalMaterial;
		m_FilterMaterials[FXAATypes::Default]	= m_FXAADefaultMaterial;
		m_FilterMaterials[FXAATypes::Fast]      = m_FXAAFastMaterial;
		m_FilterMaterials[FXAATypes::High]      = m_FXAAHighMaterial;
		m_FilterMaterials[FXAATypes::Best]      = m_FXAABestMaterial;
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

		// LineSphere
		std::vector<float> vertices;
		GenerateLineSphere(vertices, 40, 1.0f);

		// model
		m_LineModel = vk_demo::DVKModel::Create(
			m_VulkanDevice,
			cmdBuffer,
			vertices,
			{},
			{ VertexAttribute::VA_Position }
		);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_LineModel;

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

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_LineMaterial->GetPipeline());
			for (int32 j = 0; j < m_LineModel->meshes.size(); ++j) {
				m_LineMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, j);
				m_LineModel->meshes[j]->BindDrawCmd(commandBuffer);
			}

			m_RenderTarget->EndRenderPass(commandBuffer);
		}

		// fxaa pass
		{
			std::vector<VkClearValue> clearValues;
			clearValues.resize(2);
			clearValues[0].color        = { { 0.2f, 0.2f, 0.2f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo;
			ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
			renderPassBeginInfo.renderPass               = m_RenderPass;
			renderPassBeginInfo.framebuffer              = m_FrameBuffers[backBufferIndex];
			renderPassBeginInfo.clearValueCount          = clearValues.size();
			renderPassBeginInfo.pClearValues             = clearValues.data();
			renderPassBeginInfo.renderArea.offset.x      = 0;
			renderPassBeginInfo.renderArea.offset.y      = 0;
			renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
			renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			{
				vk_demo::DVKMaterial* material = m_FilterMaterials[m_Select];
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->GetPipeline());
				material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
				m_Quad->meshes[0]->BindDrawCmd(commandBuffer);
			}

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);
			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_FXAAParam.frame.x = 1.0f / m_FrameWidth;
		m_FXAAParam.frame.y = 1.0f / m_FrameHeight;

		m_ViewCamera.SetPosition(0, 0.0f, -3.0f);
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 0.1f, 1000.0f);
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
	vk_demo::DVKCamera			m_ViewCamera;
	ImageGUIContext*			m_GUI = nullptr;

	ModelViewProjectionBlock	m_MVPData;
	FXAAParamBlock				m_FXAAParam;

	vk_demo::DVKModel*			m_LineModel = nullptr;
	vk_demo::DVKShader*			m_LineShader = nullptr;
	vk_demo::DVKMaterial*		m_LineMaterial = nullptr;

	vk_demo::DVKMaterial*		m_NormalMaterial = nullptr;
	vk_demo::DVKShader*			m_NormalShader = nullptr;

	vk_demo::DVKMaterial*		m_FXAADefaultMaterial = nullptr;
	vk_demo::DVKShader*			m_FXAADefaultShader = nullptr;

	vk_demo::DVKMaterial*		m_FXAAFastMaterial = nullptr;
	vk_demo::DVKShader*			m_FXAAFastShader = nullptr;

	vk_demo::DVKMaterial*		m_FXAAHighMaterial = nullptr;
	vk_demo::DVKShader*			m_FXAAHighShader = nullptr;

	vk_demo::DVKMaterial*		m_FXAABestMaterial = nullptr;
	vk_demo::DVKShader*			m_FXAABestShader = nullptr;

	vk_demo::DVKRenderTarget*	m_RenderTarget = nullptr;
	vk_demo::DVKTexture*		m_RTColor = nullptr;
	vk_demo::DVKTexture*		m_RTDepth = nullptr;
	vk_demo::DVKModel*			m_Quad = nullptr;

	FXAATypes							m_Select = FXAATypes::Normal;
	std::vector<vk_demo::DVKMaterial*>	m_FilterMaterials;
	std::vector<const char*>			m_FilterNames;
	bool								m_AutoRotate = false;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<FXAADemo>(1400, 900, "FXAADemo", cmdLine);
}
