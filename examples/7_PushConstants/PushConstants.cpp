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

class PushConstantsMode : public AppModeBase
{
public:
	PushConstantsMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_Ready(false)
		, m_DescriptorPool(VK_NULL_HANDLE)
		, m_ImageIndex(0)
	{
        
	}

	virtual ~PushConstantsMode()
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
    
    struct PushConstants
    {
        Vector4 lightDir;
        Vector4 lightColor;
    };

	void Draw()
	{
		UpdateUniformBuffers();
		m_ImageIndex = AcquireImageIndex();
        SetupCommandBuffers();
		Present(m_ImageIndex);
	}
	
	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
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
		
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = m_FrameHeight;
		viewport.width  =  (float)m_FrameWidth;
		viewport.height = -(float)m_FrameHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = (uint32)m_FrameWidth;
		scissor.extent.height = (uint32)m_FrameHeight;
		scissor.offset.x      = 0;
		scissor.offset.y      = 0;

		VkDeviceSize offsets[1] = { 0 };
        VkPipeline pipeline = m_Material->GetPipeline(m_Renderable->GetVertexBuffer()->GetVertexInputStateInfo());

        renderPassBeginInfo.framebuffer = m_FrameBuffers[m_ImageIndex];
        VERIFYVULKANRESULT(vkBeginCommandBuffer(m_DrawCmdBuffers[m_ImageIndex], &cmdBeginInfo));
        vkCmdBeginRenderPass(m_DrawCmdBuffers[m_ImageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdSetViewport(m_DrawCmdBuffers[m_ImageIndex], 0, 1, &viewport);
        vkCmdSetScissor(m_DrawCmdBuffers[m_ImageIndex], 0, 1, &scissor);
        vkCmdBindDescriptorSets(m_DrawCmdBuffers[m_ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Shader->GetPipelineLayout(), 0, 1, &(m_DescriptorSets[m_ImageIndex]), 0, nullptr);
        vkCmdBindPipeline(m_DrawCmdBuffers[m_ImageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        
        vkCmdPushConstants(
                           m_DrawCmdBuffers[m_ImageIndex],
                           m_Shader->GetPipelineLayout(),
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                           0,
                           sizeof(PushConstants),
                           &m_LightData
                           );
        
        vkCmdBindVertexBuffers(m_DrawCmdBuffers[m_ImageIndex], 0, 1, m_Renderable->GetVertexBuffer()->GetVKBuffers().data(), offsets);
        vkCmdBindIndexBuffer(m_DrawCmdBuffers[m_ImageIndex], m_Renderable->GetIndexBuffer()->GetBuffer(), 0, m_Renderable->GetIndexBuffer()->GetIndexType());
        vkCmdDrawIndexed(m_DrawCmdBuffers[m_ImageIndex], m_Renderable->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
        vkCmdEndRenderPass(m_DrawCmdBuffers[m_ImageIndex]);
        VERIFYVULKANRESULT(vkEndCommandBuffer(m_DrawCmdBuffers[m_ImageIndex]));
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
		descriptorPoolInfo.maxSets 	 	 = GetFrameCount();
		VERIFYVULKANRESULT(vkCreateDescriptorPool(GetDevice(), &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
	}
	
	void DestroyDescriptorPool()
	{
		vkDestroyDescriptorPool(GetDevice(), m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}
    
	void UpdateUniformBuffers()
	{
        float deltaTime = Engine::Get()->GetDeltaTime();
        m_TotalTime += deltaTime;
        
        m_LightData.lightColor = Vector4(MMath::Sin(m_TotalTime), MMath::Cos(m_TotalTime), 1.0f, 0.0f);
        m_LightData.lightDir.x = MMath::Sin(m_TotalTime);
        m_LightData.lightDir.y = MMath::Cos(m_TotalTime);
        m_LightData.lightDir.z = -1;
        
        m_MVPBuffers[m_ImageIndex]->Map(sizeof(m_MVPData), 0);
        m_MVPBuffers[m_ImageIndex]->CopyTo(&m_MVPData, sizeof(m_MVPData));
        m_MVPBuffers[m_ImageIndex]->Unmap();
	}
    
	void CreateUniformBuffers()
	{
        m_TotalTime = 0.0f;
        
        m_MVPBuffers.resize(GetFrameCount());
        for (int32 i = 0; i < m_MVPBuffers.size(); ++i) {
            m_MVPBuffers[i] = VulkanBuffer::CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MVPData));
        }
        
        m_MVPData.model.SetIdentity();

		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector4(0, 0, -30.0f));
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
        m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetFrameWidth(), (float)GetFrameHeight(), 0.01f, 3000.0f);
        
		UpdateUniformBuffers();
	}

	void DestroyUniformBuffers()
	{
        for (int32 i = 0; i < m_MVPBuffers.size(); ++i)
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
        m_Shader     = Shader::Create("assets/shaders/7_PushConstants/obj.vert.spv", "assets/shaders/7_PushConstants/obj.frag.spv");
        m_Material   = std::make_shared<Material>(m_Shader);
	}
    
private:
	
	bool 							m_Ready;
    
    std::shared_ptr<Shader>         m_Shader;
	std::shared_ptr<Material>		m_Material;
	std::shared_ptr<Renderable>     m_Renderable;
    
    float                           m_TotalTime;
    PushConstants                   m_LightData;
	UBOData 						m_MVPData;
    std::vector<VulkanBuffer*>      m_MVPBuffers;
    
	std::vector<VkDescriptorSet> 	m_DescriptorSets;
	VkDescriptorPool 				m_DescriptorPool;

	uint32 							m_ImageIndex;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return new PushConstantsMode(1120, 840, "PushConstants", cmdLine);
}
