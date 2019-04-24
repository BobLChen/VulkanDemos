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
#include "Loader/tiny_obj_loader.h"
#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Data/IndexBuffer.h"
#include "Graphics/Data/VulkanBuffer.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Material/Material.h"
#include "File/FileManager.h"
#include <vector>
#include <fstream>
#include <istream>

class OBJLoaderMode : public AppModeBase
{
public:
	OBJLoaderMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_Ready(false)
		, m_VertexBuffer(nullptr)
		, m_IndexBuffer(nullptr)
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
		LoadAssets();
        CreateFences();
		CreateSemaphores();
		CreateUniformBuffers();
		CreateDescriptorPool();
        CreateDescriptorSet();
		SetupCommandBuffers();

		m_Ready = true;
	}

	virtual void Exist() override
	{
        DestroyAssets();
        DestroyFences();
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

        fenceMgr.WaitForFence(m_Fences[m_CurrentBackBuffer], MAX_uint64);
        fenceMgr.ResetFence(m_Fences[m_CurrentBackBuffer]);
        
		VkSubmitInfo submitInfo = {};
		submitInfo.sType 				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask 	= &waitStageMask;
		submitInfo.pWaitSemaphores 		= &presentCompleteSemaphore;
		submitInfo.waitSemaphoreCount 	= 1;
		submitInfo.pSignalSemaphores 	= &m_RenderComplete;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers 		= &drawCmdBuffers[m_CurrentBackBuffer];
		submitInfo.commandBufferCount 	= 1;
        
		VERIFYVULKANRESULT(vkQueueSubmit(gfxQueue->GetHandle(), 1, &submitInfo, m_Fences[m_CurrentBackBuffer]->GetHandle()));
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
            VkPipeline pipeline = m_Material->GetPipeline(m_VertexBuffer->GetVertexInputStateInfo());
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBeginInfo));
			vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Shader->GetPipelineLayout(), 0, 1, &(m_DescriptorSets[i]), 0, nullptr);
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, m_VertexBuffer->GetVKBuffers().data(), offsets);
			vkCmdBindIndexBuffer(drawCmdBuffers[i], m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(drawCmdBuffers[i], m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
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
        m_IndexBuffer  = nullptr;
        m_VertexBuffer = nullptr;
        m_Shader       = nullptr;
        m_Material     = nullptr;
        
        Material::DestroyCache();
    }
    
	void LoadAssets()
	{
		tinyobj::attrib_t 				 attrib;
		std::vector<tinyobj::shape_t> 	 shapes;
		std::vector<tinyobj::material_t> materials;
		std::string 					 warn;
		std::string 					 err;
		tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, FileManager::GetFilePath("assets/models/suzanne.obj").c_str());

		std::vector<float>  vertices;
		std::vector<uint16> indices;

		for (size_t s = 0; s < shapes.size(); ++s) 
		{
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) 
			{
				int fv = shapes[s].mesh.num_face_vertices[f];
				for (size_t v = 0; v < fv; v++) 
				{
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

					vertices.push_back(vx);
					vertices.push_back(vy);
					vertices.push_back(vz);

					vertices.push_back(nx);
					vertices.push_back(ny);
					vertices.push_back(nz);

					indices.push_back(indices.size());
				}
				index_offset += fv;
			}
		}
		
		uint8* vertStreamData = new uint8[vertices.size() * sizeof(float)];
		std::memcpy(vertStreamData, vertices.data(), vertices.size() * sizeof(float));

		VertexStreamInfo streamInfo;
		streamInfo.size 	   = vertices.size() * sizeof(float);
		streamInfo.channelMask = 1 << (int32)VertexAttribute::VA_Position | 1 << (int32)VertexAttribute::VA_Normal;

		std::vector<VertexChannelInfo> channels(2);
		channels[0].attribute = VertexAttribute::VA_Position;
		channels[0].format    = VertexElementType::VET_Float3;
		channels[0].stream    = 0;
		channels[0].offset    = 0;
		channels[1].attribute = VertexAttribute::VA_Normal;
		channels[1].format    = VertexElementType::VET_Float3;
		channels[1].stream    = 0;
		channels[1].offset    = 12;

		m_VertexBuffer = std::make_shared<VertexBuffer>();
		m_VertexBuffer->AddStream(streamInfo, channels, vertStreamData);

		// 索引数据
		uint32 indexStreamSize = indices.size() * sizeof(uint16);
		uint8* indexStreamData = new uint8[indexStreamSize];
		std::memcpy(indexStreamData, indices.data(), indexStreamSize);

		m_IndexBuffer = std::make_shared<IndexBuffer>(indexStreamData, indexStreamSize, PrimitiveType::PT_TriangleList, VkIndexType::VK_INDEX_TYPE_UINT16);
        
        m_VertexBuffer->Upload();
        m_IndexBuffer->Upload();
        
        m_Shader = Shader::Create("assets/shaders/3_OBJLoader/obj.vert.spv", "assets/shaders/3_OBJLoader/obj.frag.spv");
        m_Shader->Upload();
        m_Material = std::make_shared<Material>(m_Shader);
	}
    
    void CreateFences()
    {
        m_Fences.resize(GetBufferCount());
        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        for (int32 index = 0; index < m_Fences.size(); ++index)
        {
            m_Fences[index] = fenceMgr.CreateFence(true);
        }
    }
    
    void DestroyFences()
    {
        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        for (int32 index = 0; index < m_Fences.size(); ++index)
        {
            fenceMgr.WaitAndReleaseFence(m_Fences[index], MAX_int64);
        }
        m_Fences.clear();
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
	std::shared_ptr<VertexBuffer> 	m_VertexBuffer;
	std::shared_ptr<IndexBuffer> 	m_IndexBuffer;
    
	UBOData 						m_MVPData;
    std::vector<VulkanBuffer*>      m_MVPBuffers;
    
	VkSemaphore 					m_RenderComplete;
    std::vector<VulkanFence*> 		m_Fences;
    
	std::vector<VkDescriptorSet> 	m_DescriptorSets;
	VkDescriptorPool 				m_DescriptorPool;

	uint32 							m_CurrentBackBuffer;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return new OBJLoaderMode(800, 600, "OBJLoader", cmdLine);
}
