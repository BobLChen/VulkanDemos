#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class ComputeShaderDemo : public DemoBase
{
public:
	ComputeShaderDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~ComputeShaderDemo()
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

	struct ComputeResource
	{
		VkCommandPool					commandPool;

		VkDescriptorPool				descriptorPool;
		VkDescriptorSetLayout			descriptorSetLayout;
		VkPipelineLayout				pipelineLayout;

		VkDescriptorSet					descriptorSets[3];
		VkPipeline						pipelines[3];
		vk_demo::DVKTexture*			targets[3];
		
		void Destroy(VkDevice device)
		{
			vkDestroyCommandPool(device, commandPool, VULKAN_CPU_ALLOCATOR);
			
			vkDestroyDescriptorPool(device, descriptorPool, VULKAN_CPU_ALLOCATOR);
			vkDestroyDescriptorSetLayout(device, descriptorSetLayout, VULKAN_CPU_ALLOCATOR);
			vkDestroyPipelineLayout(device, pipelineLayout, VULKAN_CPU_ALLOCATOR);

			for (int32 i = 0; i < 3; ++i)
			{
				vkDestroyPipeline(device, pipelines[i], VULKAN_CPU_ALLOCATOR);
				delete targets[i];
			}
		}
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
			ImGui::Begin("ComputeShaderDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			if (ImGui::Combo("Filter", &m_FilterIndex, m_FilterNames.data(), m_FilterNames.size()))
			{
				if (m_FilterIndex == 0) {
					m_Material->SetTexture("diffuseMap", m_Texture);
				}
				else
				{
					m_Material->SetTexture("diffuseMap", m_ComputeRes.targets[m_FilterIndex - 1]);
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
		// https://docs.microsoft.com/zh-cn/windows/win32/direct3dhlsl/sv-dispatchthreadid
		// https://www.khronos.org/opengl/wiki/Compute_Shader
		// command pool
		{
			VkCommandPoolCreateInfo poolCreateInfo;
			ZeroVulkanStruct(poolCreateInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
			poolCreateInfo.queueFamilyIndex = m_VulkanDevice->GetComputeQueue()->GetFamilyIndex();
			poolCreateInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VERIFYVULKANRESULT(vkCreateCommandPool(m_Device, &poolCreateInfo, VULKAN_CPU_ALLOCATOR, &m_ComputeRes.commandPool));
		}

		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(
			m_VulkanDevice,
			m_ComputeRes.commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			m_VulkanDevice->GetComputeQueue()
		);
		
		// create target image
		{
			for (int32 i = 0; i < 3; ++i)
			{
				m_ComputeRes.targets[i] = vk_demo::DVKTexture::Create2D(
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
		}
        
		// DescriptorSetLayout
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings(2);
			bindings[0].binding            = 0;
			bindings[0].descriptorCount    = 1;
			bindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			bindings[0].pImmutableSamplers = nullptr;
			bindings[0].stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT; 
			bindings[1].binding            = 1;
			bindings[1].descriptorCount    = 1;
			bindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			bindings[1].pImmutableSamplers = nullptr;
			bindings[1].stageFlags         = VK_SHADER_STAGE_COMPUTE_BIT; 

			VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
			ZeroVulkanStruct(layoutCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
			layoutCreateInfo.pBindings    = bindings.data();
			layoutCreateInfo.bindingCount = bindings.size();
			VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(m_Device, &layoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_ComputeRes.descriptorSetLayout));
		}

		// PipelineLayout
		{
			VkPipelineLayoutCreateInfo layoutCreateInfo;
			ZeroVulkanStruct(layoutCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
			layoutCreateInfo.setLayoutCount = 1;
			layoutCreateInfo.pSetLayouts    = &m_ComputeRes.descriptorSetLayout;
			VERIFYVULKANRESULT(vkCreatePipelineLayout(m_Device, &layoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_ComputeRes.pipelineLayout));
		}
		
		// pool
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.descriptorCount = 2 * 3;
			poolSize.type            = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

			VkDescriptorPoolCreateInfo poolCreateInfo;
			ZeroVulkanStruct(poolCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
			poolCreateInfo.poolSizeCount = 1;
			poolCreateInfo.maxSets       = 3;
			poolCreateInfo.pPoolSizes    = &poolSize;
			VERIFYVULKANRESULT(vkCreateDescriptorPool(m_Device, &poolCreateInfo, VULKAN_CPU_ALLOCATOR, &m_ComputeRes.descriptorPool));
		}

		// DescriptorSet
		{
			VkDescriptorSetAllocateInfo allocInfo;
			ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
			allocInfo.descriptorPool     = m_ComputeRes.descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts        = &m_ComputeRes.descriptorSetLayout;
			VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_ComputeRes.descriptorSets[0]));
			VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_ComputeRes.descriptorSets[1]));
			VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_Device, &allocInfo, &m_ComputeRes.descriptorSets[2]));
		}

		// update set
		{
			for (int32 i = 0; i < 3; ++i)
			{
				VkWriteDescriptorSet writeDescriptorSet;
				ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
				writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
				writeDescriptorSet.dstSet          = m_ComputeRes.descriptorSets[i];
				writeDescriptorSet.dstBinding      = 0;
				writeDescriptorSet.descriptorCount = 1;
				writeDescriptorSet.pImageInfo      = &(m_Texture->descriptorInfo);
				vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);

				writeDescriptorSet.dstBinding      = 1;
				writeDescriptorSet.pImageInfo      = &(m_ComputeRes.targets[i]->descriptorInfo);
				vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
			}
		}

		// pipeline
		{
			VkComputePipelineCreateInfo computeCreateInfo;
			ZeroVulkanStruct(computeCreateInfo, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);
			computeCreateInfo.layout = m_ComputeRes.pipelineLayout;
			
			const char* shaderNames[3] = {
				"assets/shaders/41_ComputeShader/Contrast.comp.spv",
				"assets/shaders/41_ComputeShader/Gamma.comp.spv",
				"assets/shaders/41_ComputeShader/ColorInvert.comp.spv",
			};

			vk_demo::DVKShaderModule* shaderModules[3];

			for (int32 i = 0; i < 3; ++i)
			{
				// load shader
				shaderModules[i] = vk_demo::DVKShaderModule::Create(m_VulkanDevice, shaderNames[i], VK_SHADER_STAGE_COMPUTE_BIT);

				// stage info
				VkPipelineShaderStageCreateInfo stageInfo;
				ZeroVulkanStruct(stageInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
				stageInfo.stage  = shaderModules[i]->stage;
				stageInfo.module = shaderModules[i]->handle;
				stageInfo.pName  = "main";

				// compute info
				computeCreateInfo.stage = stageInfo;
				VERIFYVULKANRESULT(vkCreateComputePipelines(m_Device, m_PipelineCache, 1, &computeCreateInfo, VULKAN_CPU_ALLOCATOR, &(m_ComputeRes.pipelines[i])));

				delete shaderModules[i];
			}
		}

		// compute command
		cmdBuffer->Begin();

		for (int32 i = 0; i < 3; ++i)
		{
			vkCmdBindPipeline(cmdBuffer->cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputeRes.pipelines[i]);
			vkCmdBindDescriptorSets(cmdBuffer->cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_ComputeRes.pipelineLayout, 0, 1, &(m_ComputeRes.descriptorSets[i]), 0, 0);
			vkCmdDispatch(cmdBuffer->cmdBuffer, m_ComputeRes.targets[i]->width / 16, m_ComputeRes.targets[i]->height / 16, 1);
		}

		cmdBuffer->End();
		cmdBuffer->Submit();
        
        delete cmdBuffer;

		m_FilterIndex = 0;
		m_FilterNames.resize(4);
		m_FilterNames[0] = "Original";
		m_FilterNames[1] = "Contrast";
		m_FilterNames[2] = "Gamma";
		m_FilterNames[3] = "ColorInvert";
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
			"assets/shaders/41_ComputeShader/Texture.vert.spv",
			"assets/shaders/41_ComputeShader/Texture.frag.spv"
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
		m_ComputeRes.Destroy(m_Device);

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

	bool 						m_Ready = false;

	vk_demo::DVKModel*			m_ModelPlane = nullptr;
	vk_demo::DVKMaterial*		m_Material = nullptr;
	vk_demo::DVKShader*			m_Shader = nullptr;
	vk_demo::DVKTexture*		m_Texture = nullptr;

	vk_demo::DVKCamera		    m_ViewCamera;
	ModelViewProjectionBlock	m_MVPParam;

	ComputeResource				m_ComputeRes;

	std::vector<const char*>    m_FilterNames;
	int32						m_FilterIndex;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<ComputeShaderDemo>(1400, 900, "ComputeShaderDemo", cmdLine);
}
