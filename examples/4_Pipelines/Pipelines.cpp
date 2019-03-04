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
#include "Graphics/Shader/Shader.h"
#include "Graphics/Material/Material.h"
#include "File/FileManager.h"
#include <vector>

class Pipelines : public AppModeBase
{
public:
    Pipelines(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_Ready(false)
		, m_CurrentBackBuffer(0)
        , m_VertexBuffer(nullptr)
        , m_IndexBuffer(nullptr)
        , m_Shader(nullptr)
        , m_Material(nullptr)
        , m_RenderComplete(VK_NULL_HANDLE)
    {

    }
    
    virtual ~Pipelines()
    {
        
    }
    
    virtual void PreInit() override
    {
        
    }
    
    virtual void Init() override
    {
		// 准备MVP数据
		m_MVPData.model.SetIdentity();
		m_MVPData.view.SetIdentity();
		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetWidth(), (float)GetHeight(), 0.01f, 3000.0f);

		// 加载Shader以及Material
		m_Shader   = Shader::Create("assets/shaders/4_Pipelines/pipelines.vert.spv", "assets/shaders/4_Pipelines/pipelines.frag.spv");
		m_Material = std::make_shared<Material>(m_Shader);

		// 加载Mesh
        LoadOBJ();

		// 创建同步对象
		CreateSynchronousObject();

		// 录制Command命令
        SetupCommandBuffers();
        
        m_Ready = true;
    }
    
    virtual void Exist() override
    {
        DestroySynchronousObject();

        m_IndexBuffer  = nullptr;
        m_VertexBuffer = nullptr;
        m_Shader       = nullptr;
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
        
		std::shared_ptr<VulkanRHI> vulkanRHI         = GetVulkanRHI();
		VkPipelineStageFlags waitStageMask           = vulkanRHI->GetStageMask();
		std::shared_ptr<VulkanQueue> gfxQueue        = vulkanRHI->GetDevice()->GetGraphicsQueue();
		std::shared_ptr<VulkanQueue> presentQueue    = vulkanRHI->GetDevice()->GetPresentQueue();
		std::shared_ptr<VulkanSwapChain> swapChain   = vulkanRHI->GetSwapChain();
		std::vector<VkCommandBuffer>& drawCmdBuffers = vulkanRHI->GetCommandBuffers();
		VulkanFenceManager& fenceMgr                 = GetVulkanRHI()->GetDevice()->GetFenceManager();
        VkSemaphore waitSemaphore                    = VK_NULL_HANDLE;
        m_CurrentBackBuffer                          = swapChain->AcquireImageIndex(&waitSemaphore);

        fenceMgr.WaitForFence(m_Fences[m_CurrentBackBuffer], MAX_uint64);
        fenceMgr.ResetFence(m_Fences[m_CurrentBackBuffer]);
        
        VkSubmitInfo submitInfo = {};
        submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask	= &waitStageMask;
        submitInfo.pWaitSemaphores		= &waitSemaphore;
        submitInfo.waitSemaphoreCount	= 1;
        submitInfo.pSignalSemaphores	= &m_RenderComplete;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pCommandBuffers		= &drawCmdBuffers[m_CurrentBackBuffer];
        submitInfo.commandBufferCount	= 1;
        
        VERIFYVULKANRESULT(vkQueueSubmit(gfxQueue->GetHandle(), 1, &submitInfo, m_Fences[m_CurrentBackBuffer]->GetHandle()));
        swapChain->Present(gfxQueue, presentQueue, &m_RenderComplete);
    }
    
    void SetupCommandBuffers()
    {
		std::shared_ptr<VulkanRHI> vulkanRHI = GetVulkanRHI();
        
        VkCommandBufferBeginInfo cmdBeginInfo;
        ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
        
        VkClearValue clearValues[2];
        clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        
        VkRenderPassBeginInfo renderPassBeginInfo;
        ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = vulkanRHI->GetRenderPass();
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues    = clearValues;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width  = vulkanRHI->GetSwapChain()->GetWidth();
        renderPassBeginInfo.renderArea.extent.height = vulkanRHI->GetSwapChain()->GetHeight();
        
        std::vector<VkCommandBuffer>& drawCmdBuffers = vulkanRHI->GetCommandBuffers();
        std::vector<VkFramebuffer>& frameBuffers     = vulkanRHI->GetFrameBuffers();
        
        for (int32 i = 0; i < drawCmdBuffers.size(); ++i)
        {
            renderPassBeginInfo.framebuffer = frameBuffers[i];
            
            VkViewport viewport = {};
            viewport.width    = (float)renderPassBeginInfo.renderArea.extent.width;
            viewport.height   = (float)renderPassBeginInfo.renderArea.extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            
            VkRect2D scissor = {};
            scissor.extent.width  = (uint32)viewport.width;
            scissor.extent.height = (uint32)viewport.height;
            scissor.offset.x      = 0;
            scissor.offset.y      = 0;
            
            VkDeviceSize offsets[1] = { 0 };
            
            VERIFYVULKANRESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBeginInfo));
            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

			if (!m_IndexBuffer->IsValid())
			{
				continue;
			}

			if (!m_VertexBuffer->IsValid())
			{
				continue;
			}

			VkPipeline pipeline = m_Material->GetPipeline(m_VertexBuffer->GetVertexInputStateInfo(), m_Shader->GetVertexInputBindingInfo());

            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Shader->GetPipelineLayout(), 0, 1, &(m_Shader->GetDescriptorSet()), 0, nullptr);
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, m_VertexBuffer->GetVKBuffers().data(), offsets);
            vkCmdBindIndexBuffer(drawCmdBuffers[i], m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(drawCmdBuffers[i], m_IndexBuffer->GetIndexCount(), 1, 0, 0, 1);
            
			vkCmdEndRenderPass(drawCmdBuffers[i]);
            VERIFYVULKANRESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }
    
    void UpdateUniformBuffers()
    {
		m_MVPData.model.AppendRotation(0.1f, Vector3::UpVector);

        m_MVPData.view.SetIdentity();
        m_MVPData.view.SetOrigin(Vector4(0, 0, 30.0f));
        m_MVPData.view.SetInverse();
        
        m_Shader->SetUniformData("uboMVP", (uint8*)&m_MVPData, sizeof(UBOData));
    }
    
    void LoadOBJ()
    {
        tinyobj::attrib_t                attrib;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string                      warn;
        std::string                      err;
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
                    //tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    //tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                    
                    vertices.push_back(vx);
                    vertices.push_back(-vy);
                    vertices.push_back(vz);
                    
                    vertices.push_back(nx);
                    vertices.push_back(-ny);
                    vertices.push_back(nz);
                    
                    indices.push_back(indices.size());
                }
                index_offset += fv;
            }
        }
        
        uint8* vertStreamData = new uint8[vertices.size() * sizeof(float)];
        std::memcpy(vertStreamData, vertices.data(), vertices.size() * sizeof(float));
        
        VertexStreamInfo       streamInfo;
        streamInfo.size        = vertices.size() * sizeof(float);
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
    }
    
    void CreateSynchronousObject()
    {
		std::shared_ptr<VulkanRHI> vulkanRHI = GetVulkanRHI();
		VkDevice device = vulkanRHI->GetDevice()->GetInstanceHandle();

        m_Fences.resize(GetVulkanRHI()->GetSwapChain()->GetBackBufferCount());
        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        for (int32 index = 0; index < m_Fences.size(); ++index)
        {
            m_Fences[index] = fenceMgr.CreateFence(true);
        }
        
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		vkCreateSemaphore(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
    }
    
    void DestroySynchronousObject()
    {
		std::shared_ptr<VulkanRHI> vulkanRHI = GetVulkanRHI();
		VkDevice device = vulkanRHI->GetDevice()->GetInstanceHandle();

        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        for (int32 index = 0; index < m_Fences.size(); ++index)
        {
            fenceMgr.WaitAndReleaseFence(m_Fences[index], MAX_int64);
        }
        m_Fences.clear();

		vkDestroySemaphore(device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
    }

private:
    UBOData                       m_MVPData;
    bool                          m_Ready;
    uint32                        m_CurrentBackBuffer;

	std::shared_ptr<VertexBuffer> m_VertexBuffer;
	std::shared_ptr<IndexBuffer>  m_IndexBuffer;
    std::shared_ptr<Shader>       m_Shader;
	std::shared_ptr<Material>     m_Material;
    
    VkSemaphore                   m_RenderComplete;
    std::vector<VulkanFence*>     m_Fences;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
    return new Pipelines(800, 600, "Pipelines", cmdLine);
}
