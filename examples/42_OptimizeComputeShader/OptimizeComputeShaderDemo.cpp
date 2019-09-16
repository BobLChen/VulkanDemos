#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class OptimizeComputeShaderDemo : public DemoBase
{
public:
	OptimizeComputeShaderDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~OptimizeComputeShaderDemo()
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
		ProcessImage();

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

		SetupCommandBuffers(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("OptimizeComputeShaderDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			if (ImGui::Combo("Filter", &m_FilterIndex, m_FilterNames.data(), m_FilterNames.size()))
			{
				if (m_FilterIndex == 0) {
					m_Material->SetTexture("diffuseMap", m_Texture);
				}
				else
				{
					m_Material->SetTexture("diffuseMap", m_ComputeTargets[m_FilterIndex - 1]);
				}
			}

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void ProcessImage()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(
			m_VulkanDevice, 
			m_ComputeCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			m_VulkanDevice->GetComputeQueue()
		);

		// create target image
        for (int32 i = 0; i < 3; ++i)
        {
            m_ComputeTargets[i] = vk_demo::DVKTexture::Create2D(
                m_VulkanDevice,
                cmdBuffer,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_IMAGE_ASPECT_COLOR_BIT,
                m_Texture->width, m_Texture->height,
                VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
                VK_SAMPLE_COUNT_1_BIT,
                ImageLayoutBarrier::ComputeGeneralRW
            );
        }
        
        const char* shaderNames[3] = {
            "assets/shaders/42_OptimizeComputeShader/Contrast.comp.spv",
            "assets/shaders/42_OptimizeComputeShader/Gamma.comp.spv",
            "assets/shaders/42_OptimizeComputeShader/ColorInvert.comp.spv",
        };
        
        for (int32 i = 0; i < 3; ++i)
        {
            m_ComputeShaders[i]    = vk_demo::DVKShader::Create(m_VulkanDevice, shaderNames[i]);
            m_ComputeProcessors[i] = vk_demo::DVKCompute::Create(m_VulkanDevice, m_PipelineCache, m_ComputeShaders[i]);
            m_ComputeProcessors[i]->SetStorageTexture("inputImage", m_Texture);
            m_ComputeProcessors[i]->SetStorageTexture("outputImage", m_ComputeTargets[i]);
        }
        
		// compute command
		cmdBuffer->Begin();

		for (int32 i = 0; i < 3; ++i)
		{
			m_ComputeProcessors[i]->BindDispatch(cmdBuffer->cmdBuffer, m_ComputeTargets[i]->width / 16, m_ComputeTargets[i]->height / 16, 1);
		}

		cmdBuffer->End();
		cmdBuffer->Submit();

		m_FilterIndex = 0;
		m_FilterNames.resize(4);
		m_FilterNames[0] = "Original";
		m_FilterNames[1] = "Contrast";
		m_FilterNames[2] = "Gamma";
		m_FilterNames[3] = "ColorInvert";
        
        delete cmdBuffer;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_ModelPlane = vk_demo::DVKModel::LoadFromFile(
			"assets/models/plane_z.obj",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position, 
				VertexAttribute::VA_UV0
			}
		);
		m_ModelPlane->rootNode->localMatrix.AppendScale(Vector3(2, 1, 1));

		m_Texture = vk_demo::DVKTexture::Create2D(
			"assets/textures/game0.jpg", 
			m_VulkanDevice, 
			cmdBuffer,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_Shader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/42_OptimizeComputeShader/Texture.vert.spv",
			"assets/shaders/42_OptimizeComputeShader/Texture.frag.spv"
		);

		m_Material = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader
		);
		m_Material->pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_NONE;
		m_Material->PreparePipeline();
		m_Material->SetTexture("diffuseMap", m_Texture);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
        for (int32 i = 0; i < 3; ++i)
        {
            delete m_ComputeShaders[i];
            delete m_ComputeTargets[i];
            delete m_ComputeProcessors[i];
        }
        
		delete m_ModelPlane;
		delete m_Texture;

		delete m_Material;
		delete m_Shader;
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

		m_Material->BeginFrame();
		for (int32 i = 0; i < m_ModelPlane->meshes.size(); ++i)
		{
			m_MVPParam.model = m_ModelPlane->meshes[i]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = m_ViewCamera.GetView();
			m_MVPParam.proj  = m_ViewCamera.GetProjection();

			m_Material->BeginObject();
			m_Material->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
			m_Material->EndObject();

			m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, i);
			m_ModelPlane->meshes[i]->BindDrawCmd(commandBuffer);
		}
		m_Material->EndFrame();

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);
		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.SetPosition(0, 0, -2.5f);
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

	vk_demo::DVKModel*			    m_ModelPlane = nullptr;
	vk_demo::DVKMaterial*		    m_Material = nullptr;
	vk_demo::DVKShader*			    m_Shader = nullptr;
	vk_demo::DVKTexture*		    m_Texture = nullptr;

	vk_demo::DVKCamera		        m_ViewCamera;
	ModelViewProjectionBlock	    m_MVPParam;
    
    vk_demo::DVKTexture*            m_ComputeTargets[3];
    vk_demo::DVKShader*             m_ComputeShaders[3];
    vk_demo::DVKCompute*   m_ComputeProcessors[3];
    
	std::vector<const char*>        m_FilterNames;
	int32						    m_FilterIndex;

	ImageGUIContext*			    m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<OptimizeComputeShaderDemo>(1400, 900, "OptimizeComputeShaderDemo", cmdLine);
}
