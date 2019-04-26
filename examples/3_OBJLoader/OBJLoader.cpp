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
#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Data/IndexBuffer.h"
#include "Graphics/Data/VulkanBuffer.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Renderer/Renderable.h"
#include "File/FileManager.h"
#include "Loader/MeshLoader.h"

#include <vector>
#include <fstream>
#include <istream>

class OBJLoaderMode : public AppModeBase
{
public:
	OBJLoaderMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_Ready(false)
		, m_RenderComplete(VK_NULL_HANDLE)
		, m_DescriptorPool(VK_NULL_HANDLE)
		, m_CurrentBackBuffer(0)
	{
        
	}

	virtual ~OBJLoaderMode()
	{

	}

	virtual void PreInit() override
	{
		
	}

	virtual void Init() override
	{
		Prepare();
		LoadAssets();
		CreateSemaphores();
		CreateUniformBuffers();
		CreateDescriptorPool();
        CreateDescriptorSet();
		SetupCommandBuffers();

		m_Ready = true;
	}

	virtual void Exist() override
	{
		WaitFences(m_CurrentBackBuffer);

		Release();
        DestroyAssets();
		DestorySemaphores();
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

		VkPipelineStageFlags waitStageMask 			 = GetVulkanRHI()->GetStageMask();
		std::shared_ptr<VulkanQueue> gfxQueue 		 = GetVulkanRHI()->GetDevice()->GetGraphicsQueue();
		std::shared_ptr<VulkanQueue> presentQueue    = GetVulkanRHI()->GetDevice()->GetPresentQueue();
		std::vector<VkCommandBuffer>& drawCmdBuffers = GetVulkanRHI()->GetCommandBuffers();
		std::shared_ptr<VulkanSwapChain> swapChain   = GetVulkanRHI()->GetSwapChain();
		VkSemaphore presentCompleteSemaphore 		 = VK_NULL_HANDLE;
		m_CurrentBackBuffer 						 = swapChain->AcquireImageIndex(&presentCompleteSemaphore);
        VulkanFenceManager& fenceMgr 				 = GetVulkanRHI()->GetDevice()->GetFenceManager();

		WaitFences(m_CurrentBackBuffer);
        
		VkSubmitInfo submitInfo = {};
		submitInfo.sType 				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask 	= &waitStageMask;
		submitInfo.pWaitSemaphores 		= &presentCompleteSemaphore;
		submitInfo.waitSemaphoreCount 	= 1;
		submitInfo.pSignalSemaphores 	= &m_RenderComplete;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers 		= &drawCmdBuffers[m_CurrentBackBuffer];
		submitInfo.commandBufferCount 	= 1;
        
		VERIFYVULKANRESULT(vkQueueSubmit(gfxQueue->GetHandle(), 1, &submitInfo, m_Fences[m_CurrentBackBuffer]));
		swapChain->Present(gfxQueue, presentQueue, &m_RenderComplete);
	}
	
	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };
        
        int32 width  = GetRealWidth();
        int32 height = GetRealHeight();
        
		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass      = GetVulkanRHI()->GetRenderPass();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues    = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width  = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		
		std::vector<VkCommandBuffer>& drawCmdBuffers = GetVulkanRHI()->GetCommandBuffers();
		std::vector<VkFramebuffer> frameBuffers      = GetVulkanRHI()->GetFrameBuffers();
		for (int32 i = 0; i < drawCmdBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VkViewport viewport = {};
			viewport.x = 0;
			viewport.y = height;
			viewport.width  = (float)width;
			viewport.height = -(float)height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.extent.width  = (uint32)width;
			scissor.extent.height = (uint32)height;
			scissor.offset.x      = 0;
			scissor.offset.y      = 0;

			VkDeviceSize offsets[1] = { 0 };
            VkPipeline pipeline = m_Material->GetPipeline(m_Renderable->GetVertexBuffer()->GetVertexInputStateInfo());
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Shader->GetPipelineLayout(), 0, 1, &(m_DescriptorSets[i]), 0, nullptr);
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, m_Renderable->GetVertexBuffer()->GetVKBuffers().data(), offsets);
			vkCmdBindIndexBuffer(drawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetBuffer(), 0, m_Renderable->GetIndexBuffer()->GetIndexType());
			vkCmdDrawIndexed(drawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
			vkCmdEndRenderPass(drawCmdBuffers[i]);
			VERIFYVULKANRESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
		}
	}
    
	void CreateDescriptorSet()
	{
		m_DescriptorSets.resize(GetVulkanRHI()->GetSwapChain()->GetBackBufferCount());

		for (int i = 0; i < m_DescriptorSets.size(); ++i)
		{
			VkDescriptorSetAllocateInfo allocInfo;
			ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
			allocInfo.descriptorPool     = m_DescriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts        = &(m_Shader->GetDescriptorSetLayout());
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
    
	void CreateDescriptorPool()
	{
        const std::vector<VkDescriptorPoolSize>& poolSize = m_Material->GetShader()->GetPoolSizes();
        
		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
        descriptorPoolInfo.poolSizeCount = (uint32_t)poolSize.size();
        descriptorPoolInfo.pPoolSizes    = poolSize.size() > 0 ? poolSize.data() : nullptr;
		descriptorPoolInfo.maxSets 	 	 = GetVulkanRHI()->GetSwapChain()->GetBackBufferCount();
		VERIFYVULKANRESULT(vkCreateDescriptorPool(GetDevice(), &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
	}
	
	void DestroyDescriptorPool()
	{
		vkDestroyDescriptorPool(GetDevice(), m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}
    
	void UpdateUniformBuffers()
	{
        float deltaTime = Engine::Get()->GetDeltaTime();
        m_MVPData.model.AppendRotation(90.0f * deltaTime, Vector3::UpVector);
        
        m_MVPBuffers[m_CurrentBackBuffer]->Map(sizeof(m_MVPData), 0);
        m_MVPBuffers[m_CurrentBackBuffer]->CopyTo(&m_MVPData, sizeof(m_MVPData));
        m_MVPBuffers[m_CurrentBackBuffer]->Unmap();
	}
    
	void CreateUniformBuffers()
	{
        m_MVPBuffers.resize(GetBufferCount());
        for (int i = 0; i < m_MVPBuffers.size(); ++i) {
            m_MVPBuffers[i] = VulkanBuffer::CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MVPData));
        }
        
        m_MVPData.model.SetIdentity();

		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector4(0, 0, -30.0f));
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
        m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetRealWidth(), (float)GetRealHeight(), 0.01f, 3000.0f);
        
		UpdateUniformBuffers();
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
        m_Shader       = nullptr;
        m_Material     = nullptr;
        m_Renderable   = nullptr;

        Material::DestroyCache();
    }
    
	void LoadAssets()
	{
        m_Renderable = MeshLoader::LoadFromFile("assets/models/suzanne.obj")[0];
        m_Shader     = Shader::Create("assets/shaders/3_OBJLoader/obj.vert.spv", "assets/shaders/3_OBJLoader/obj.frag.spv");
        m_Material   = std::make_shared<Material>(m_Shader);
	}
    
	void CreateSemaphores()
	{
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		vkCreateSemaphore(GetDevice(), &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
	}
    
	void DestorySemaphores()
	{
		vkDestroySemaphore(GetDevice(), m_RenderComplete, VULKAN_CPU_ALLOCATOR);
	}

private:
	
	bool 							m_Ready;
    
    std::shared_ptr<Shader>         m_Shader;
	std::shared_ptr<Material>		m_Material;
	std::shared_ptr<Renderable>     m_Renderable;

	UBOData 						m_MVPData;
    std::vector<VulkanBuffer*>      m_MVPBuffers;
    
	VkSemaphore 					m_RenderComplete;
    
	std::vector<VkDescriptorSet> 	m_DescriptorSets;
	VkDescriptorPool 				m_DescriptorPool;

	uint32 							m_CurrentBackBuffer;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return new OBJLoaderMode(800, 600, "OBJLoader", cmdLine);
}
