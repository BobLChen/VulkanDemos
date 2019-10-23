#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"

#include <vector>

class OptimizeShaderAndLayoutModuleDemo : public DemoBase
{
public:
	OptimizeShaderAndLayoutModuleDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}
    
	virtual ~OptimizeShaderAndLayoutModuleDemo()
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
		CreateGUI();
		CreateUniformBuffers();
        CreateDescriptorSet();
		CreatePipelines();
		SetupCommandBuffers();

		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
		DemoBase::Release();

		DestroyAssets();
		DestroyGUI();
		DestroyPipelines();
		DestroyUniformBuffers();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

private:

	struct ImageInfo
	{
		int32	width  = 0;
		int32	height = 0;
		int32	comp   = 0;
		uint8*	data   = nullptr;
	};
    
    struct LutDebugBlock
    {
        float bias;
        float padding0;
        float padding1;
        float padding2;
    };
    
	struct MVPBlock
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

		UpdateUniformBuffers(time, delta);
		
		DemoBase::Present(bufferIndex);
	}
    
	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();
        
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Texture3D", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("3D LUT");
            
            ImGui::SliderFloat("DebugLut", &m_LutDebugData.bias, 0.0f, 1.0f);
            
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
		}
        
		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
        
		if (m_GUI->Update()) {
			SetupCommandBuffers();
		}

		return hovered;
	}

	void LoadAssets()
	{
		// 创建Shader
		m_ShaderTexture = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/16_OptimizeShaderAndLayout/texture.vert.spv",
			"assets/shaders/16_OptimizeShaderAndLayout/texture.frag.spv"
		);
		m_ShaderLut = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/16_OptimizeShaderAndLayout/lut.vert.spv",
			"assets/shaders/16_OptimizeShaderAndLayout/lut.frag.spv"
		);
		m_ShaderLutDebug0 = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/16_OptimizeShaderAndLayout/debug0.vert.spv",
			"assets/shaders/16_OptimizeShaderAndLayout/debug0.frag.spv"
		);
		m_ShaderLutDebug1 = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/16_OptimizeShaderAndLayout/debug1.vert.spv",
			"assets/shaders/16_OptimizeShaderAndLayout/debug1.frag.spv"
		);

		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// 读取模型文件
		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/plane_z.obj",
			m_VulkanDevice,
			cmdBuffer,
			m_ShaderTexture->perVertexAttributes
		);
        
		// 生成LUT 3D图数据
		// 64mb 
		// map image0 -> image1
		int32 lutSize  = 256;
		uint8* lutRGBA = new uint8[lutSize * lutSize * 4 * lutSize];
        for (int32 x = 0; x < lutSize; ++x)
        {
            for (int32 y = 0; y < lutSize; ++y)
            {
                for (int32 z = 0; z < lutSize; ++z)
                {
                    int idx = (x + y * lutSize + z * lutSize * lutSize) * 4;
                    int32 r = x * 1.0f / (lutSize - 1) * 255;
                    int32 g = y * 1.0f / (lutSize - 1) * 255;
                    int32 b = z * 1.0f / (lutSize - 1) * 255;
                    // 怀旧PS滤镜，色调映射。
                    r = 0.393f * r + 0.769f * g + 0.189f * b;
                    g = 0.349f * r + 0.686f * g + 0.168f * b;
                    b = 0.272f * r + 0.534f * g + 0.131f * b;
                    lutRGBA[idx + 0] = MMath::Min(r, 255);
                    lutRGBA[idx + 1] = MMath::Min(g, 255);
                    lutRGBA[idx + 2] = MMath::Min(b, 255);
                    lutRGBA[idx + 3] = 255;
                }
            }
        }
        
		// 创建Texture
		m_TexOrigin = vk_demo::DVKTexture::Create2D("assets/textures/game0.jpg", m_VulkanDevice, cmdBuffer);
		m_Tex3DLut  = vk_demo::DVKTexture::Create3D(VK_FORMAT_R8G8B8A8_UNORM, lutRGBA, lutSize * lutSize * 4 * lutSize, lutSize, lutSize, lutSize, m_VulkanDevice, cmdBuffer);
		
		delete cmdBuffer;
	}
    
	void DestroyAssets()
	{
		delete m_Model;

		delete m_TexOrigin;
        delete m_Tex3DLut;

		delete m_ShaderTexture;
		delete m_ShaderLut;
		delete m_ShaderLutDebug0;
		delete m_ShaderLutDebug1;
	}
    
	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color        = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = m_RenderPass;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues    = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
        renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;
        
		int32 ww = m_FrameWidth  / 2.0f;
		int32 hh = m_FrameHeight / 2.0f;

		VkViewport viewport = {};
        viewport.x        = 0;
        viewport.y        = hh;
        viewport.width    = ww;
        viewport.height   = -(float)hh;    // flip y axis
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
		
        VkRect2D scissor = {};
        scissor.extent.width  = ww;
        scissor.extent.height = hh;
        scissor.offset.x      = 0;
        scissor.offset.y      = 0;

		for (int32 i = 0; i < m_CommandBuffers.size(); ++i)
		{
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            
			// 0
			viewport.x = 0;
			viewport.y = hh;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);
            vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipeline);
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipelineLayout, 0, m_DescriptorSet0->descriptorSets.size(), m_DescriptorSet0->descriptorSets.data(), 0, nullptr);
            for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
                m_Model->meshes[meshIndex]->BindDrawCmd(m_CommandBuffers[i]);
            }

			// 1
			viewport.x = ww;
			viewport.y = hh;
			scissor.offset.x = ww;
			scissor.offset.y = 0;
			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);
            vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipeline);
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipelineLayout, 0, m_DescriptorSet1->descriptorSets.size(), m_DescriptorSet1->descriptorSets.data(), 0, nullptr);
            for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
                m_Model->meshes[meshIndex]->BindDrawCmd(m_CommandBuffers[i]);
            }

			// 2
			viewport.x = 0;
			viewport.y = hh * 2;
			scissor.offset.x = 0;
			scissor.offset.y = hh;
			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);
            vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline2->pipeline);
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline2->pipelineLayout, 0, m_DescriptorSet2->descriptorSets.size(), m_DescriptorSet2->descriptorSets.data(), 0, nullptr);
            for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
                m_Model->meshes[meshIndex]->BindDrawCmd(m_CommandBuffers[i]);
            }
            
			// 3
			viewport.x = ww;
			viewport.y = hh * 2;
			scissor.offset.x = ww;
			scissor.offset.y = hh;
			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);
            vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline3->pipeline);
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline3->pipelineLayout, 0, m_DescriptorSet3->descriptorSets.size(), m_DescriptorSet3->descriptorSets.data(), 0, nullptr);
            for (int32 meshIndex = 0; meshIndex < m_Model->meshes.size(); ++meshIndex) {
                m_Model->meshes[meshIndex]->BindDrawCmd(m_CommandBuffers[i]);
            }
			
			m_GUI->BindDrawCmd(m_CommandBuffers[i], m_RenderPass);

			vkCmdEndRenderPass(m_CommandBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
		}
	}
    
	void CreateDescriptorSet()
	{
		m_DescriptorSet0 = m_ShaderTexture->AllocateDescriptorSet();
		m_DescriptorSet0->WriteBuffer("uboMVP", m_MVPBuffer);
		m_DescriptorSet0->WriteImage("diffuseMap", m_TexOrigin);

		m_DescriptorSet1 = m_ShaderLut->AllocateDescriptorSet();
		m_DescriptorSet1->WriteBuffer("uboMVP", m_MVPBuffer);
		m_DescriptorSet1->WriteImage("diffuseMap", m_TexOrigin);
		m_DescriptorSet1->WriteImage("lutMap", m_Tex3DLut);

		m_DescriptorSet2 = m_ShaderLutDebug0->AllocateDescriptorSet();
		m_DescriptorSet2->WriteBuffer("uboMVP", m_MVPBuffer);
		m_DescriptorSet2->WriteImage("diffuseMap", m_TexOrigin);
		m_DescriptorSet2->WriteImage("lutMap", m_Tex3DLut);
		m_DescriptorSet2->WriteBuffer("uboLutDebug", m_LutDebugBuffer);

		m_DescriptorSet3 = m_ShaderLutDebug1->AllocateDescriptorSet();
		m_DescriptorSet3->WriteBuffer("uboMVP", m_MVPBuffer);
		m_DescriptorSet3->WriteImage("diffuseMap", m_TexOrigin);
		m_DescriptorSet3->WriteImage("lutMap", m_Tex3DLut);
		m_DescriptorSet3->WriteBuffer("uboLutDebug", m_LutDebugBuffer);
	}
    
	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();
		
		vk_demo::DVKGfxPipelineInfo pipelineInfo0;
		pipelineInfo0.shader = m_ShaderTexture;
		m_Pipeline0 = vk_demo::DVKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo0, { vertexInputBinding }, vertexInputAttributs, m_ShaderTexture->pipelineLayout, m_RenderPass);
        
        vk_demo::DVKGfxPipelineInfo pipelineInfo1;
		pipelineInfo1.shader = m_ShaderLut;
        m_Pipeline1 = vk_demo::DVKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo1, { vertexInputBinding }, vertexInputAttributs, m_ShaderLut->pipelineLayout, m_RenderPass);
        
        vk_demo::DVKGfxPipelineInfo pipelineInfo2;
		pipelineInfo2.shader = m_ShaderLutDebug0;
        m_Pipeline2 = vk_demo::DVKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo2, { vertexInputBinding }, vertexInputAttributs, m_ShaderLutDebug0->pipelineLayout, m_RenderPass);
        
        vk_demo::DVKGfxPipelineInfo pipelineInfo3;
		pipelineInfo3.shader = m_ShaderLutDebug1;
        m_Pipeline3 = vk_demo::DVKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo3, { vertexInputBinding }, vertexInputAttributs, m_ShaderLutDebug1->pipelineLayout, m_RenderPass);
	}
    
	void DestroyPipelines()
	{
        delete m_Pipeline0;
        delete m_Pipeline1;
        delete m_Pipeline2;
        delete m_Pipeline3;

		delete m_DescriptorSet0;
		delete m_DescriptorSet1;
		delete m_DescriptorSet2;
		delete m_DescriptorSet3;
	}
	
	void UpdateUniformBuffers(float time, float delta)
	{
		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();
		m_MVPBuffer->CopyFrom(&m_MVPData, sizeof(MVPBlock));

        m_LutDebugBuffer->CopyFrom(&m_LutDebugData, sizeof(LutDebugBlock));
	}
    
	void CreateUniformBuffers()
	{
		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;
		boundCenter.z = -1.0f;

		m_MVPData.model.AppendRotation(180, Vector3::UpVector);
		m_MVPData.model.AppendScale(Vector3(1.0f, 0.5f, 1.0f));

		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 0.1f, 1500.0f);
		m_ViewCamera.SetPosition(boundCenter);

		m_MVPBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(MVPBlock),
			&(m_MVPData)
		);
		m_MVPBuffer->Map();
        
        // lut debug data
		m_LutDebugData.bias = 0.0f;
        m_LutDebugBuffer = vk_demo::DVKBuffer::CreateBuffer(
           m_VulkanDevice,
           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
           sizeof(LutDebugBlock),
           &(m_LutDebugData)
        );
        m_LutDebugBuffer->Map();
	}
	
	void DestroyUniformBuffers()
	{
		m_MVPBuffer->UnMap();
		delete m_MVPBuffer;
        
        m_LutDebugBuffer->UnMap();
        delete m_LutDebugBuffer;
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

	bool 							m_Ready = false;
    
	vk_demo::DVKCamera				m_ViewCamera;

	MVPBlock 						m_MVPData;
	vk_demo::DVKBuffer*				m_MVPBuffer;
    
    LutDebugBlock                   m_LutDebugData;
    vk_demo::DVKBuffer*             m_LutDebugBuffer = nullptr;;
    
	vk_demo::DVKTexture*			m_TexOrigin = nullptr;
	vk_demo::DVKTexture*			m_Tex3DLut  = nullptr;
	
    vk_demo::DVKGfxPipeline*        m_Pipeline0 = nullptr;
    vk_demo::DVKGfxPipeline*        m_Pipeline1 = nullptr;
    vk_demo::DVKGfxPipeline*        m_Pipeline2 = nullptr;
    vk_demo::DVKGfxPipeline*        m_Pipeline3 = nullptr;
    
	vk_demo::DVKShader*				m_ShaderTexture = nullptr;
	vk_demo::DVKShader*				m_ShaderLut = nullptr;
	vk_demo::DVKShader*				m_ShaderLutDebug0 = nullptr;
	vk_demo::DVKShader*				m_ShaderLutDebug1 = nullptr;

	vk_demo::DVKModel*				m_Model = nullptr;

	vk_demo::DVKDescriptorSet*		m_DescriptorSet0 = nullptr;
	vk_demo::DVKDescriptorSet*		m_DescriptorSet1 = nullptr;
	vk_demo::DVKDescriptorSet*		m_DescriptorSet2 = nullptr;
	vk_demo::DVKDescriptorSet*		m_DescriptorSet3 = nullptr;
	
	ImageGUIContext*				m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<OptimizeShaderAndLayoutModuleDemo>(1400, 900, "OptimizeShaderAndLayoutModuleDemo", cmdLine);
}
