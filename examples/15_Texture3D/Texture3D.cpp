#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"
#include "Demo/DVKTexture.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"
#include "File/FileManager.h"
#include "UI/ImageGUIContext.h"

#include <vector>
#include <fstream>

class Texture3DModule : public DemoBase
{
public:
	Texture3DModule(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
        
	}
    
	virtual ~Texture3DModule()
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
    
	struct MVPBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	void Draw(float time, float delta)
	{
        UpdateUI(time, delta);
		UpdateUniformBuffers(time, delta);
        DemoBase::Present();
	}
    
	void UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();
        
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
            ImGui::Begin("Texture3D", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text("3D LUT");

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
		}
        
		m_GUI->EndFrame();
        
		if (m_GUI->Update()) {
			SetupCommandBuffers();
		}
	}

	bool LoadImage(const std::string& filename, ImageInfo& imageInfo)
	{
		uint32 dataSize = 0;
        uint8* dataPtr  = nullptr;
        if (!FileManager::ReadFile(filename, dataPtr, dataSize)) {
            MLOGE("Failed load image : %s", filename.c_str());
            return false;
        }

		int32 comp   = 0;
        int32 width  = 0;
        int32 height = 0;
        uint8* rgbaData = StbImage::LoadFromMemory(dataPtr, dataSize, &width, &height, &comp, 4);
        uint32 size  = width * height * 4;
        
		delete[] dataPtr;
		dataPtr = nullptr;

        if (rgbaData == nullptr) {
            MLOGE("Failed load image : %s", filename.c_str());
            return false;
        }

		imageInfo.comp   = 4;
		imageInfo.data   = rgbaData;
		imageInfo.width  = width;
		imageInfo.height = height;

		return true;
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

		ImageInfo image0;
		LoadImage("assets/textures/game0.jpg", image0);
		ImageInfo image1;
		LoadImage("assets/textures/game0_filter.jpg", image1);

		// 64mb 
		// map image0 -> image1
		int32 lutSize  = 256;
		uint8* lutRGBA = new uint8[lutSize * lutSize * 4 * lutSize];
		for (int32 i = 0; i < image0.width; ++i)
		{
			for (int32 j = 0; j < image0.height; ++j)
			{
				int32 idx = j * image0.width * 4 + i * 4;
				uint8 r0  = image0.data[idx + 0];
				uint8 g0  = image0.data[idx + 1];
				uint8 b0  = image0.data[idx + 2];

				uint8 r1  = image1.data[idx + 0];
				uint8 g1  = image1.data[idx + 1];
				uint8 b1  = image1.data[idx + 2];

				// x:r;y:g;z:b
				idx = r0 * 4 + g0 * lutSize * 4 + b0 * lutSize * lutSize * 4;
				lutRGBA[idx + 0] = r1;
				lutRGBA[idx + 1] = g1;
				lutRGBA[idx + 2] = b1;
				lutRGBA[idx + 3] = 255;
			}
		}

		m_TexOrigin = vk_demo::DVKTexture::Create2D(image0.data, image0.width, image0.height, m_VulkanDevice, cmdBuffer);
		m_TexFilter = vk_demo::DVKTexture::Create2D(image1.data, image1.width, image1.height, m_VulkanDevice, cmdBuffer);
		m_Tex3DLut  = vk_demo::DVKTexture::Create3D(VK_FORMAT_R8G8B8A8_UNORM, lutRGBA, lutSize * lutSize * 4 * lutSize, lutSize, lutSize, lutSize, m_VulkanDevice, cmdBuffer);
		
		StbImage::Free(image0.data);
		StbImage::Free(image1.data);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;
		delete m_TexOrigin;
		delete m_TexFilter;
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
            
            vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->pipeline);
            
			// 0
			viewport.x = 0;
			viewport.y = hh;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet0, 0, nullptr);
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
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet1, 0, nullptr);
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
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet2, 0, nullptr);
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
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet3, 0, nullptr);
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
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;
        
		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 2;
		descriptorPoolInfo.pPoolSizes    = poolSizes;
		descriptorPoolInfo.maxSets       = 4;
		VERIFYVULKANRESULT(vkCreateDescriptorPool(m_Device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
        
		// set0
		std::vector<VkDescriptorSet*> descriptorSets = {
			&m_DescriptorSet0,
			&m_DescriptorSet1,
			&m_DescriptorSet2,
			&m_DescriptorSet3
		};
		std::vector<vk_demo::DVKTexture*> textures = {
			m_TexOrigin,
			m_TexFilter,
			m_TexOrigin,
			m_TexFilter
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
		}
	}
    
	void CreatePipelines()
	{
		VkVertexInputBindingDescription vertexInputBinding = m_Model->GetInputBinding();
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs = m_Model->GetInputAttributes();
		
		vk_demo::DVKPipelineInfo pipelineInfo(m_VulkanDevice);
        pipelineInfo.vertShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/texture.vert.spv");
		pipelineInfo.fragShaderModule = vk_demo::LoadSPIPVShader(m_Device, "assets/shaders/15_Texture3D/texture.frag.spv");
		m_Pipeline0 = vk_demo::DVKPipeline::Create(m_VulkanDevice, m_PipelineCache, pipelineInfo, { vertexInputBinding }, vertexInputAttributs, m_PipelineLayout, m_RenderPass);
	}
    
	void DestroyPipelines()
	{
        delete m_Pipeline0;
        m_Pipeline0 = nullptr;
	}
	
	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layoutBindings[2] = { };
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
		
		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 2;
		descSetLayoutInfo.pBindings    = layoutBindings;
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

	}
    
	void CreateUniformBuffers()
	{
		vk_demo::DVKBoundingBox bounds = m_Model->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
        Vector3 boundCenter = bounds.min + boundSize * 0.5f;
		boundCenter.z = -10.0f;

		m_MVPData.model.SetIdentity();
		m_MVPData.model.SetOrigin(Vector3(0, 0, 0));
		m_MVPData.model.AppendScale(Vector3(1.0f, 0.5f, 1.0f));
        
		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(boundCenter);
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(MMath::DegreesToRadians(75.0f), (float)GetWidth() / 2, (float)GetHeight() / 2, 0.01f, 3000.0f);
		
		m_MVPBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(MVPBlock),
			&(m_MVPData)
		);
		m_MVPBuffer->Map();
	}
	
	void DestroyUniformBuffers()
	{
		m_MVPBuffer->UnMap();
		delete m_MVPBuffer;
		m_MVPBuffer = nullptr;
	}

	void CreateGUI()
	{
		m_GUI = new ImageGUIContext();
		m_GUI->Init("assets/fonts/Roboto-Medium.ttf");
	}

	void DestroyGUI()
	{
		m_GUI->Destroy();
		delete m_GUI;
	}

private:

	bool 							m_Ready = false;
    
	MVPBlock 						m_MVPData;
	vk_demo::DVKBuffer*				m_MVPBuffer;

	vk_demo::DVKTexture*			m_TexOrigin = nullptr;
	vk_demo::DVKTexture*			m_TexFilter = nullptr;
	vk_demo::DVKTexture*			m_Tex3DLut  = nullptr;
	
    vk_demo::DVKPipeline*           m_Pipeline0 = nullptr;

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
	return std::make_shared<Texture3DModule>(1400, 900, "Texture3D", cmdLine);
}
