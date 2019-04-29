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
#include "Graphics/Texture/Texture2D.h"
#include "File/FileManager.h"
#include <vector>

class TextureMode : public AppModeBase
{
public:
	TextureMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_Ready(false)
		, m_ImageIndex(0)
	{

	}

	virtual ~TextureMode()
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

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		int32 width  = GetFrameWidth();
		int32 height = GetFrameHeight();

		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
		renderPassBeginInfo.renderPass = GetVulkanRHI()->GetRenderPass();
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;

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
        
		for (int32 i = 0; i < drawCmdBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = frameBuffers[i];
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

			writeDescriptorSet.dstSet 		   = m_DescriptorSets[i];
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.pImageInfo      = &(m_Diffuse->GetDescriptorImageInfo());
			writeDescriptorSet.dstBinding      = 1;
			vkUpdateDescriptorSets(GetDevice(), 1, &writeDescriptorSet, 0, nullptr);

			writeDescriptorSet.dstSet 		   = m_DescriptorSets[i];
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.pImageInfo      = &(m_Specular->GetDescriptorImageInfo());
			writeDescriptorSet.dstBinding      = 2;
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

	void CreateUniformBuffers()
	{
		m_MVPBuffers.resize(GetFrameCount());
        for (int32 i = 0; i < m_MVPBuffers.size(); ++i) {
            m_MVPBuffers[i] = VulkanBuffer::CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(m_MVPData));
        }
        
        m_MVPData.model.SetIdentity();

		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector4(0, 0, -10.0f));
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
        m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetFrameWidth(), (float)GetFrameHeight(), 0.01f, 3000.0f);
        
		UpdateUniformBuffers();
	}

	void DestroyAssets()
	{
		m_Shader     = nullptr;
        m_Material   = nullptr;
        m_Renderable = nullptr;

        Material::DestroyCache();
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

	void UpdateUniformBuffers()
	{
		float deltaTime = Engine::Get()->GetDeltaTime();
        m_MVPData.model.AppendRotation(90.0f * deltaTime, Vector3::UpVector);
        
        m_MVPBuffers[m_ImageIndex]->Map(sizeof(m_MVPData), 0);
        m_MVPBuffers[m_ImageIndex]->CopyTo(&m_MVPData, sizeof(m_MVPData));
        m_MVPBuffers[m_ImageIndex]->Unmap();
	}

	void LoadAssets()
	{
		m_Diffuse  = std::make_shared<Texture2D>();
		m_Diffuse->LoadFromFile("assets/textures/head_diffuse.jpg");
		m_Specular = std::make_shared<Texture2D>();
		m_Specular->LoadFromFile("assets/textures/head_specular.jpg");

		m_Shader   = Shader::Create("assets/shaders/5_Texture/diffuse.vert.spv", "assets/shaders/5_Texture/diffuse.frag.spv");
		m_Material = std::make_shared<Material>(m_Shader);
		
		m_Renderable = MeshLoader::LoadFromFile("assets/models/head.obj")[0];
	}

private:
	bool							m_Ready;

	std::shared_ptr<Shader>         m_Shader;
	std::shared_ptr<Material>		m_Material;
	std::shared_ptr<Renderable>     m_Renderable;

	std::shared_ptr<Texture2D>		m_Diffuse;
	std::shared_ptr<Texture2D>		m_Specular;

	UBOData 						m_MVPData;
    std::vector<VulkanBuffer*>      m_MVPBuffers;
	
	std::vector<VkDescriptorSet> 	m_DescriptorSets;
	VkDescriptorPool 				m_DescriptorPool;

	uint32 							m_ImageIndex;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return new TextureMode(1120, 840, "Texture", cmdLine);
}
