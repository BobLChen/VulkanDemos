#include "Common/Common.h"
#include "Common/Log.h"
#include "Configuration/Platform.h"
#include "Application/AppModeBase.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanQueue.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanMemory.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Loader/MeshLoader.h"
#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Data/IndexBuffer.h"
#include "Graphics/Data/VulkanBuffer.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Renderer/Mesh.h"
#include "Graphics/Renderer/Renderable.h"
#include "File/FileManager.h"
#include <vector>

typedef std::shared_ptr<Renderable>		RenderablePtr;
typedef std::shared_ptr<Shader>			ShaderPtr;
typedef std::shared_ptr<Material>		MaterialPtr;

class PipelinesMode : public AppModeBase
{
public:
    PipelinesMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_Ready(false)
		, m_ImageIndex(0)
    {
        
    }
    
    virtual ~PipelinesMode()
    {
        
    }
    
    virtual void PreInit() override
    {
        
    }
    
    virtual void Init() override
    {
		Prepare();
        LoadAssets();
		CreateUniformBuffers();
		CreateDescriptorPool();
        CreateDescriptorSet();
        SetupCommandBuffers();
        m_Ready = true;
    }
    
    virtual void Exist() override
    {
		WaitFences(m_ImageIndex);
		Release();
        DestroyAssets();
		DestroyUniformBuffers();
		DestroyDescriptorPool();
    }
    
    virtual void Loop() override
    {
        if (m_Ready)
        {
			Draw();
        }
    }
    
private:
    
    struct UBOData
    {
        Matrix4x4 model;
        Matrix4x4 view;
        Matrix4x4 projection;
    };
    
    void Draw()
    {
		UpdateUniformBuffers();
		m_ImageIndex = AcquireImageIndex();
		Present(m_ImageIndex);
    }

    void SetupCommandBuffers()
    {
		std::vector<VkCommandBuffer>& drawCmdBuffers = GetVulkanRHI()->GetCommandBuffers();
		std::vector<VkFramebuffer>& frameBuffers     = GetVulkanRHI()->GetFrameBuffers();

		uint32 width  = GetFrameWidth();
		uint32 height = GetFrameHeight();

		VkCommandBufferBeginInfo cmdBeginInfo;
        ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

        VkClearValue clearValues[2];
        clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo;
        ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = GetVulkanRHI()->GetRenderPass();
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues    = clearValues;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width  = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        
        for (int32 i = 0; i < drawCmdBuffers.size(); ++i)
        {
            renderPassBeginInfo.framebuffer = frameBuffers[i];
            
            VERIFYVULKANRESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBeginInfo));
            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            
			for (int32 j = 0; j < m_Shaders.size(); ++j)
			{
				int32 ww = 0.5f * width;
				int32 hh = 0.5f * height;
				int32 tx = (j % 2) * ww;
				int32 ty = (j / 2) * hh;

				VkViewport viewport = {};
				viewport.width  = ww;
				viewport.height = -hh;
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				viewport.x = tx;
				viewport.y = ty + hh;

				VkRect2D scissor = {};
				scissor.extent.width  = width;
				scissor.extent.height = height;
				scissor.offset.x = tx;
				scissor.offset.y = ty;

				VkDeviceSize offsets[1] = { 0 };
				VkPipeline pipeline = m_Materials[j]->GetPipeline(m_Renderable->GetVertexBuffer()->GetVertexInputStateInfo());
				
				vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
				vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
				vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Shaders[0]->GetPipelineLayout(), 0, 1, &(m_DescriptorSets[i]), 0, nullptr);
				vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
				vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, m_Renderable->GetVertexBuffer()->GetVKBuffers().data(), offsets);
				vkCmdBindIndexBuffer(drawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetBuffer(), 0, m_Renderable->GetIndexBuffer()->GetIndexType());
				vkCmdDrawIndexed(drawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
			}

			vkCmdEndRenderPass(drawCmdBuffers[i]);
            VERIFYVULKANRESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }

	void CreateDescriptorSet()
	{
		m_DescriptorSets.resize(GetFrameCount());

		for (int32 i = 0; i < m_DescriptorSets.size(); ++i)
		{
			VkDescriptorSetAllocateInfo allocInfo;
			ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
			allocInfo.descriptorPool     = m_DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts        = &(m_Shaders[0]->GetDescriptorSetLayout());
			VERIFYVULKANRESULT(vkAllocateDescriptorSets(GetDevice(), &allocInfo, &(m_DescriptorSets[i])));
			
			VkWriteDescriptorSet writeDescriptorSet;
			ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
			writeDescriptorSet.dstSet 		   = m_DescriptorSets[i];
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.pBufferInfo     = &(m_MVPBuffers[i]->GetDescriptorBufferInfo());
			writeDescriptorSet.dstBinding      = 0;
			vkUpdateDescriptorSets(GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
		}
	}

	void DestroyDescriptorPool()
	{
		vkDestroyDescriptorPool(GetDevice(), m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}

	void CreateDescriptorPool()
	{
		// 随便找个Shader获取PoolSize，都一样
		const std::vector<VkDescriptorPoolSize>& poolSize = m_Shaders[0]->GetPoolSizes();
        
		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
        descriptorPoolInfo.poolSizeCount = (uint32_t)poolSize.size();
        descriptorPoolInfo.pPoolSizes    = poolSize.size() > 0 ? poolSize.data() : nullptr;
		descriptorPoolInfo.maxSets 	 	 = GetFrameCount();
		VERIFYVULKANRESULT(vkCreateDescriptorPool(GetDevice(), &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
	}

	void CreateUniformBuffers()
	{
		m_MVPBuffers.resize(GetFrameCount());
		for (int32 i = 0; i < m_MVPBuffers.size(); ++i) {
			m_MVPBuffers[i] = VulkanBuffer::CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MVPData));
		}

		m_MVPData.model.SetIdentity();

		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector4(0, 0, -30.0f));
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetWidth(), (float)GetHeight(), 0.01f, 3000.0f);
	}
    
    void UpdateUniformBuffers()
    {
		float deltaTime = Engine::Get()->GetDeltaTime();
        m_MVPData.model.AppendRotation(90.0f * deltaTime, Vector3::UpVector);
        
        m_MVPBuffers[m_ImageIndex]->Map(sizeof(m_MVPData), 0);
        m_MVPBuffers[m_ImageIndex]->CopyTo(&m_MVPData, sizeof(m_MVPData));
        m_MVPBuffers[m_ImageIndex]->Unmap();
    }

	void DestroyUniformBuffers()
	{
		for (int i = 0; i < m_MVPBuffers.size(); ++i)
        {
            m_MVPBuffers[i]->Destroy();
            delete m_MVPBuffers[i];
        }
        m_MVPBuffers.clear();
	}

	void DestroyAssets()
	{
		m_Shaders.clear();
		m_Materials.clear();
		m_Renderable = nullptr;
		Material::DestroyCache();
	}
    
    void LoadAssets()
    {
		m_Shaders.resize(4);
		m_Materials.resize(4);

		// 加载Shader以及Material
		m_Shaders[0]   = Shader::Create("assets/shaders/4_Pipelines/phong.vert.spv", "assets/shaders/4_Pipelines/phong.frag.spv");
		m_Materials[0] = std::make_shared<Material>(m_Shaders[0]);
		
		m_Shaders[1]   = Shader::Create("assets/shaders/4_Pipelines/pipelines.vert.spv", "assets/shaders/4_Pipelines/pipelines.frag.spv");
		m_Materials[1] = std::make_shared<Material>(m_Shaders[1]);
		
		m_Shaders[2]   = Shader::Create("assets/shaders/4_Pipelines/solid.vert.spv", "assets/shaders/4_Pipelines/solid.frag.spv");
		m_Materials[2] = std::make_shared<Material>(m_Shaders[2]);
		
		m_Shaders[3]   = Shader::Create("assets/shaders/4_Pipelines/solid.vert.spv", "assets/shaders/4_Pipelines/solid.frag.spv");
		m_Materials[3] = std::make_shared<Material>(m_Shaders[3]);
		m_Materials[3]->SetpolygonMode(VkPolygonMode::VK_POLYGON_MODE_LINE);
		
		m_Renderable = MeshLoader::LoadFromFile("assets/models/suzanne.obj")[0];
    }
    
private:
	bool							m_Ready;

	std::vector<ShaderPtr>			m_Shaders;
	std::vector<MaterialPtr>		m_Materials;
	std::shared_ptr<Renderable>		m_Renderable;

    UBOData							m_MVPData;
	std::vector<VulkanBuffer*>		m_MVPBuffers;

	std::vector<VkDescriptorSet>	m_DescriptorSets;
	VkDescriptorPool 				m_DescriptorPool;
    
    uint32							m_ImageIndex;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
    return new PipelinesMode(1120, 840, "Pipelines", cmdLine);
}
