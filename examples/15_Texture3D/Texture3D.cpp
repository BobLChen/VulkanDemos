#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"

#include <vector>

class Texture3DDemo : public DemoBase
{
public:
	Texture3DDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}
    
	virtual ~Texture3DDemo()
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
		CreateDescriptorSetLayout();
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
        DestroyDescriptorSetLayout();
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
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/plane_z.obj",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position, 
				VertexAttribute::VA_UV0
			}
		);
        
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
        
		m_TexOrigin = vk_demo::DVKTexture::Create2D("assets/textures/game0.jpg", m_VulkanDevice, cmdBuffer);
		m_Tex3DLut  = vk_demo::DVKTexture::Create3D(VK_FORMAT_R8G8B8A8_UNORM, lutRGBA, lutSize * lutSize * 4 * lutSize, lutSize, lutSize, lutSize, m_VulkanDevice, cmdBuffer);
		
		delete cmdBuffer;
	}
    
	void DestroyAssets()
	{
		delete m_Model;
		delete m_TexOrigin;
        delete m_Tex3DLut;
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
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipelineLayout, 0, 1, &m_DescriptorSet0, 0, nullptr);
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
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->pipelineLayout, 0, 1, &m_DescriptorSet1, 0, nullptr);
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
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline2->pipelineLayout, 0, 1, &m_DescriptorSet2, 0, nullptr);
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
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline3->pipelineLayout, 0, 1, &m_DescriptorSet3, 0, nullptr);
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
		VkDescriptorPoolSize poolSizes[2];
		poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 2;
		poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 2;
        
		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 2;
		descriptorPoolInfo.pPoolSizes    = poolSizes;
		descriptorPoolInfo.maxSets       = 4;
		VERIFYVULKANRESULT(vkCreateDescriptorPool(m_Device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
        
		std::vector<VkDescriptorSet*> descriptorSets = {
			&m_DescriptorSet0,
			&m_DescriptorSet1,
			&m_DescriptorSet2,
			&m_DescriptorSet3
		};
		std::vector<vk_demo::DVKTexture*> textures = {
			m_TexOrigin,
			m_TexOrigin,
			m_TexOrigin,
			m_TexOrigin
		};
        
		for (int32 i = 0; i < descriptorSets.size(); ++i)
		{
			VkDescriptorSetAllocateInfo allocInfo;
			ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
			allocInfo.descriptorPool     = m_DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts        = &m_DescriptorSetLayout;
			VERIFYVULKANRESULT(vkAllocateDescriptorSets(m_Device, &allocInfo, descriptorSets[i]));
			
			VkWriteDescriptorSet writeDescriptorSet;
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet          = *(descriptorSets[i]);
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.pBufferInfo     = &(m_MVPBuffer->descriptor);
			writeDescriptorSet.dstBinding      = 0;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
        
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet          = *(descriptorSets[i]);
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.pBufferInfo     = nullptr;
			writeDescriptorSet.pImageInfo      = &(textures[i]->descriptorInfo);
			writeDescriptorSet.dstBinding      = 1;
			vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
            
            ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
            writeDescriptorSet.dstSet          = *(descriptorSets[i]);
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptorSet.pBufferInfo     = nullptr;
            writeDescriptorSet.pImageInfo      = &(m_Tex3DLut->descriptorInfo);
            writeDescriptorSet.dstBinding      = 2;
            vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
            
            ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
            writeDescriptorSet.dstSet          = *(descriptorSets[i]);
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSet.pBufferInfo     = &(m_LutDebugBuffer->descriptor);
            writeDescriptorSet.dstBinding      = 3;
            vkUpdateDescriptorSets(m_Device, 1, &writeDescriptorSet, 0, nullptr);
		}
	}
    
	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();
		
		vk_demo::DVKGfxPipelineInfo pipelineInfo0;
        pipelineInfo0.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/texture.vert.spv");
		pipelineInfo0.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/texture.frag.spv");
		m_Pipeline0 = vk_demo::DVKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo0, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);
        
        vk_demo::DVKGfxPipelineInfo pipelineInfo1;
        pipelineInfo1.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/lut.vert.spv");
        pipelineInfo1.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/lut.frag.spv");
        m_Pipeline1 = vk_demo::DVKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo1, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);
        
        vk_demo::DVKGfxPipelineInfo pipelineInfo2;
        pipelineInfo2.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/debug0.vert.spv");
        pipelineInfo2.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/debug0.frag.spv");
        m_Pipeline2 = vk_demo::DVKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo2, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);
        
        vk_demo::DVKGfxPipelineInfo pipelineInfo3;
        pipelineInfo3.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/debug1.vert.spv");
        pipelineInfo3.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/debug1.frag.spv");
        m_Pipeline3 = vk_demo::DVKGfxPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo3, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);
		
		vkDestroyShaderModule(m_Device, pipelineInfo0.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo0.fragShaderModule, VULKAN_CPU_ALLOCATOR);

		vkDestroyShaderModule(m_Device, pipelineInfo1.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo1.fragShaderModule, VULKAN_CPU_ALLOCATOR);

		vkDestroyShaderModule(m_Device, pipelineInfo2.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo2.fragShaderModule, VULKAN_CPU_ALLOCATOR);

		vkDestroyShaderModule(m_Device, pipelineInfo3.vertShaderModule, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(m_Device, pipelineInfo3.fragShaderModule, VULKAN_CPU_ALLOCATOR);
	}
    
	void DestroyPipelines()
	{
        delete m_Pipeline0;
        delete m_Pipeline1;
        delete m_Pipeline2;
        delete m_Pipeline3;
	}
	
	void CreateDescriptorSetLayout()
	{
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings(4);
		layoutBindings[0].binding 			 = 0;
		layoutBindings[0].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBindings[0].descriptorCount    = 1;
		layoutBindings[0].stageFlags 		 = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBindings[0].pImmutableSamplers = nullptr;

		layoutBindings[1].binding 			 = 1;
		layoutBindings[1].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutBindings[1].descriptorCount    = 1;
		layoutBindings[1].stageFlags 		 = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutBindings[1].pImmutableSamplers = nullptr;
        
        layoutBindings[2].binding            = 2;
        layoutBindings[2].descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[2].descriptorCount    = 1;
        layoutBindings[2].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[2].pImmutableSamplers = nullptr;
        
        layoutBindings[3].binding            = 3;
        layoutBindings[3].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        layoutBindings[3].descriptorCount    = 1;
        layoutBindings[3].stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[3].pImmutableSamplers = nullptr;
        
		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = layoutBindings.size();
		descSetLayoutInfo.pBindings    = layoutBindings.data();
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(m_Device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));
        
		VkPipelineLayoutCreateInfo pipeLayoutInfo;
		ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipeLayoutInfo.setLayoutCount = 1;
		pipeLayoutInfo.pSetLayouts    = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkCreatePipelineLayout(m_Device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
	}
    
	void DestroyDescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(m_Device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyDescriptorPool(m_Device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
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
		m_LutDebugData.bias = 0;
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
    
	vk_demo::DVKModel*				m_Model = nullptr;

	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
	
	VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;

	VkDescriptorSet 				m_DescriptorSet0 = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet1 = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet2 = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet3 = VK_NULL_HANDLE;
	
	ImageGUIContext*				m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<Texture3DDemo>(1400, 900, "Texture3D", cmdLine);
}
