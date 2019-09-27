#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

#define PARTICLE_COUNT (1024 * 1024)

class ComputeParticlesDemo : public DemoBase
{
public:
	ComputeParticlesDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~ComputeParticlesDemo()
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

	struct ParticleVertex
	{
		Vector4 position;
		Vector4 velocity;
	};

	struct ParticleParam
	{
		Vector4 data0;
		Vector4 data1;
	};
    
	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);
		UpdateUI(time, delta);

		SetupComputeCommand();

		SetupGfxCommand(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("ComputeParticlesDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::SliderInt("Count", &m_PointCount, PARTICLE_COUNT / 10, PARTICLE_COUNT);

			ImGui::SliderFloat("PointSize", &m_ParticleParams.data1.x, 1.0f, 15.0f);
			ImGui::SliderFloat("Intensity", &m_ParticleParams.data1.y, 0.1f, 1.0f);
            ImGui::SliderFloat("Range",     &m_ParticleParams.data0.z, 0.0001f, 0.01f, "%5f");
			ImGui::SliderFloat("Drag",      &m_ParticleParams.data1.z, 0.0f, 1.0f);
			ImGui::SliderFloat("Ease",      &m_ParticleParams.data1.w, 0.0f, 1.0f);

			ImGui::Checkbox("Mouse", &m_Animation);

			if (m_Animation)
			{
				const Vector2& mousePos = InputManager::GetMousePosition();
				float dx = mousePos.x / GetWidth();
				float dy = mousePos.y / GetHeight();
				dx = (dx - 0.5f) * 2.0f;
				dy = -(dy - 0.5f) * 2.0f;
				m_ParticleParams.data0.x = dx;
				m_ParticleParams.data0.y = dy;
			}
			else
			{
                m_ParticleParams.data0.x = MMath::Sin(time * time * 0.01);
                m_ParticleParams.data0.y = MMath::Cos(time);
			}
            
			m_ParticleParams.data0.w = PARTICLE_COUNT;

			m_ComputeProcessor->SetUniform("param", &m_ParticleParams, sizeof(ParticleParam));

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
		m_ComputeCommand = vk_demo::DVKCommandBuffer::Create(
			m_VulkanDevice, 
			m_ComputeCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			m_VulkanDevice->GetComputeQueue()
		);

		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		{
			std::vector<ParticleVertex> vertices(PARTICLE_COUNT);
			for (int32 i = 0; i < PARTICLE_COUNT; ++i)
			{
				vertices[i].position.x = MMath::FRandRange(-1.0f, 1.0f);
				vertices[i].position.y = MMath::FRandRange(-1.0f, 1.0f);
				vertices[i].position.z = vertices[i].position.x;
				vertices[i].position.w = vertices[i].position.y;

				vertices[i].velocity.x = 0.0f;
				vertices[i].velocity.y = 0.0f;
				vertices[i].velocity.z = 0.0f;
				vertices[i].velocity.w = (vertices[i].position.x + 1.0f) / 2.0f;
			}

			vk_demo::DVKBuffer* stagingBuffer = vk_demo::DVKBuffer::CreateBuffer(
				m_VulkanDevice, 
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				vertices.size() * sizeof(ParticleVertex), 
				vertices.data()
			);

			m_ParticleBuffer = vk_demo::DVKBuffer::CreateBuffer(
				m_VulkanDevice, 
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				vertices.size() * sizeof(ParticleVertex)
			);

			cmdBuffer->Begin();

			VkBufferCopy copyRegion = {};
			copyRegion.size = vertices.size() * sizeof(ParticleVertex);
			vkCmdCopyBuffer(cmdBuffer->cmdBuffer, stagingBuffer->buffer, m_ParticleBuffer->buffer, 1, &copyRegion);

			cmdBuffer->End();
			cmdBuffer->Submit();

			delete stagingBuffer;
		}

		m_GradientTexture = vk_demo::DVKTexture::Create2D(
			"assets/textures/gradient.png", 
			m_VulkanDevice, 
			cmdBuffer,
			VK_IMAGE_USAGE_SAMPLED_BIT,
			ImageLayoutBarrier::PixelShaderRead
		);

		m_DiffuseTexture = vk_demo::DVKTexture::Create2D(
			"assets/textures/particle.png", 
			m_VulkanDevice, 
			cmdBuffer,
			VK_IMAGE_USAGE_SAMPLED_BIT,
			ImageLayoutBarrier::PixelShaderRead
		);

		m_ParticleShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/43_ComputeParticles/Particle.vert.spv",
			"assets/shaders/43_ComputeParticles/Particle.frag.spv"
		);

		m_ParticleMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_ParticleShader
		);
		m_ParticleMaterial->pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_NONE;
		m_ParticleMaterial->pipelineInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		m_ParticleMaterial->pipelineInfo.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		m_ParticleMaterial->pipelineInfo.inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		m_ParticleMaterial->pipelineInfo.depthStencilState.depthTestEnable = VK_FALSE;
		m_ParticleMaterial->pipelineInfo.depthStencilState.depthWriteEnable = VK_FALSE;
		m_ParticleMaterial->pipelineInfo.depthStencilState.stencilTestEnable = VK_FALSE;
		m_ParticleMaterial->pipelineInfo.depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].blendEnable = VK_TRUE;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		m_ParticleMaterial->PreparePipeline();
		m_ParticleMaterial->SetTexture("diffuseMap",  m_DiffuseTexture);
		m_ParticleMaterial->SetTexture("gradientMap", m_GradientTexture);

		m_ComputeShader = vk_demo::DVKShader::Create(
			m_VulkanDevice, 
			"assets/shaders/43_ComputeParticles/Particle.comp.spv"
		);

		m_ComputeProcessor = vk_demo::DVKCompute::Create(
			m_VulkanDevice, 
			m_PipelineCache, 
			m_ComputeShader
		);
		m_ComputeProcessor->SetStorageBuffer("inVertex", m_ParticleBuffer);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_ParticleBuffer;
		delete m_ParticleMaterial;
		delete m_ParticleShader;

		delete m_GradientTexture;
		delete m_DiffuseTexture;

		delete m_ComputeShader;
		delete m_ComputeProcessor;

		delete m_ComputeCommand;
	}

	void SetupComputeCommand()
	{
		m_ComputeCommand->Begin();

		VkBufferMemoryBarrier bufferBarrier;
		ZeroVulkanStruct(bufferBarrier, VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER);
		bufferBarrier.buffer = m_ParticleBuffer->buffer;
		bufferBarrier.size   = m_ParticleBuffer->size;
		bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;						
		bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;																											
		bufferBarrier.srcQueueFamilyIndex = m_VulkanDevice->GetGraphicsQueue()->GetFamilyIndex();
		bufferBarrier.dstQueueFamilyIndex = m_VulkanDevice->GetComputeQueue()->GetFamilyIndex();

		vkCmdPipelineBarrier(
			m_ComputeCommand->cmdBuffer,
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0, nullptr,
			1, &bufferBarrier,
			0, nullptr
		);

		m_ComputeProcessor->BindDispatch(m_ComputeCommand->cmdBuffer, 32, 32, 1);

		bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;						
		bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;																											
		bufferBarrier.srcQueueFamilyIndex = m_VulkanDevice->GetComputeQueue()->GetFamilyIndex();
		bufferBarrier.dstQueueFamilyIndex = m_VulkanDevice->GetGraphicsQueue()->GetFamilyIndex();

		vkCmdPipelineBarrier(
			m_ComputeCommand->cmdBuffer,
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			VK_PIPELINE_STAGE_VERTEX_SHADER_BIT,
			0,
			0, nullptr,
			1, &bufferBarrier,
			0, nullptr
		);

		m_ComputeCommand->Submit();
	}

	void SetupGfxCommand(int32 backBufferIndex)
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

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ParticleMaterial->GetPipeline());

		VkDeviceSize offsets[1] = { 0 };

		m_ParticleMaterial->BeginFrame();
		m_ParticleMaterial->BeginObject();
		m_ParticleMaterial->SetLocalUniform("param", &m_ParticleParams, sizeof(ParticleParam));
		m_ParticleMaterial->EndObject();

		m_ParticleMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(m_ParticleBuffer->buffer), offsets);
		vkCmdDraw(commandBuffer, m_PointCount, 1, 0, 0);

		m_ParticleMaterial->EndFrame();

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);
		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ParticleParams.data0.x = 0.0f;
		m_ParticleParams.data0.y = 0.0f;
		m_ParticleParams.data0.z = 0.0001f;
		m_ParticleParams.data0.w = PARTICLE_COUNT;
        
		m_ParticleParams.data1.x = 8.0f;
		m_ParticleParams.data1.y = 0.5f;
		m_ParticleParams.data1.z = 0.95f;
		m_ParticleParams.data1.w = 0.25f;

		m_PointCount = PARTICLE_COUNT / 2;
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

	vk_demo::DVKBuffer*				m_ParticleBuffer = nullptr;
	vk_demo::DVKShader*				m_ParticleShader = nullptr;
	vk_demo::DVKMaterial*			m_ParticleMaterial = nullptr;

	vk_demo::DVKTexture*			m_GradientTexture = nullptr;
	vk_demo::DVKTexture*			m_DiffuseTexture = nullptr;

    vk_demo::DVKShader*             m_ComputeShader = nullptr;
    vk_demo::DVKCompute*   			m_ComputeProcessor = nullptr;
	vk_demo::DVKCommandBuffer*		m_ComputeCommand = nullptr;

	ParticleParam					m_ParticleParams;
	int32							m_PointCount = 0;
	bool							m_Animation = false;
    
	ImageGUIContext*			    m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<ComputeParticlesDemo>(1400, 900, "ComputeParticlesDemo", cmdLine);
}
