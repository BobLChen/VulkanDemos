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
#include "Loader/OBJMeshParser.h"
#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Data/IndexBuffer.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Renderer/Mesh.h"
#include "Graphics/Renderer/Renderable.h"
#include "File/FileManager.h"
#include <vector>

typedef std::shared_ptr<Mesh>		MeshPtr;
typedef std::shared_ptr<Shader>		ShaderPtr;
typedef std::shared_ptr<Material>	MaterialPtr;

class Pipelines : public AppModeBase
{
public:
    Pipelines(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModeBase(width, height, title)
		, m_Ready(false)
		, m_CurrentBackBuffer(0)
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

		// 加载Mesh
        LoadAssets();

		// 创建同步对象
		CreateSynchronousObject();

		// 录制Command命令
        SetupCommandBuffers();
        
        m_Ready = true;
    }
    
    virtual void Exist() override
    {
        DestroySynchronousObject();
		m_Meshes.clear();
        Material::DestroyCache();
    }
    
    virtual void Loop() override
    {
        if (m_Ready)
        {
            UpdateUniformBuffers();
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

	void BindMeshCommand(std::shared_ptr<Mesh> mesh, VkCommandBuffer command)
	{
		std::vector<std::shared_ptr<Renderable>> renderables = mesh->GetRenderables();
		std::vector<std::shared_ptr<Material>> materials = mesh->GetMaterials();

		for (int32 i = 0; i < renderables.size(); ++i)
		{
			std::shared_ptr<Renderable> renderable = renderables[i];
			std::shared_ptr<Material> material = materials[i];
			std::shared_ptr<Shader> shader = material->GetShader();

			if (!renderable->IsValid())
			{
				continue;
			}
			
			const VertexInputDeclareInfo& vertInfo = renderable->GetVertexBuffer()->GetVertexInputStateInfo();
			VkPipeline pipeline = material->GetPipeline(vertInfo, shader->GetVertexInputBindingInfo());

			vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetPipelineLayout(), 0, 1, &(shader->GetDescriptorSet()), 0, nullptr);
			vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			
			renderable->BindBufferToCommand(command);
			renderable->BindDrawToCommand(command);
		}

	}
    
    void SetupCommandBuffers()
    {
		std::shared_ptr<VulkanRHI> vulkanRHI = GetVulkanRHI();
        
        VkCommandBufferBeginInfo cmdBeginInfo;
        ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
        
        VkClearValue clearValues[2];
        clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

		uint32 width  = vulkanRHI->GetSwapChain()->GetWidth();
		uint32 height = vulkanRHI->GetSwapChain()->GetHeight();
        
        VkRenderPassBeginInfo renderPassBeginInfo;
        ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = vulkanRHI->GetRenderPass();
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues    = clearValues;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width  = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        
        std::vector<VkCommandBuffer>& drawCmdBuffers = vulkanRHI->GetCommandBuffers();
        std::vector<VkFramebuffer>& frameBuffers     = vulkanRHI->GetFrameBuffers();
        
        for (int32 i = 0; i < drawCmdBuffers.size(); ++i)
        {
            renderPassBeginInfo.framebuffer = frameBuffers[i];
            
            VERIFYVULKANRESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBeginInfo));
            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            
			for (int32 j = 0; j < m_Meshes.size(); ++j)
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

				vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
				vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

				BindMeshCommand(m_Meshes[j], drawCmdBuffers[i]);
			}

			vkCmdEndRenderPass(drawCmdBuffers[i]);
            VERIFYVULKANRESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }
    
    void UpdateUniformBuffers()
    {
		m_MVPData.model.AppendRotation(0.1f, Vector3::UpVector);

        m_MVPData.view.SetIdentity();
        m_MVPData.view.SetOrigin(Vector4(0, -2.5f, -30.0f));

        m_MVPData.view.AppendRotation(15.0f, Vector3::RightVector);
        m_MVPData.view.SetInverse();
        
		for (int32 i = 0; i < m_Meshes.size(); ++i)
		{
			const std::vector<MaterialPtr>& materials = m_Meshes[i]->GetMaterials();
			for (int32 j = 0; j < materials.size(); ++j)
			{
				materials[j]->GetShader()->SetUniformData("uboMVP", (uint8*)&m_MVPData, sizeof(UBOData));
			}
		}
    }
    
    void LoadAssets()
    {
		// 加载Shader以及Material
		ShaderPtr   shader0 = Shader::Create("assets/shaders/4_Pipelines/phong.vert.spv", "assets/shaders/4_Pipelines/phong.frag.spv");
		MaterialPtr material0 = std::make_shared<Material>(shader0);
		ShaderPtr	shader1 = Shader::Create("assets/shaders/4_Pipelines/pipelines.vert.spv", "assets/shaders/4_Pipelines/pipelines.frag.spv");
		MaterialPtr material1 = std::make_shared<Material>(shader1);
		ShaderPtr	shader2 = Shader::Create("assets/shaders/4_Pipelines/solid.vert.spv", "assets/shaders/4_Pipelines/solid.frag.spv");
		MaterialPtr	material2 = std::make_shared<Material>(shader2);
		ShaderPtr	shader3 = Shader::Create("assets/shaders/4_Pipelines/solid.vert.spv", "assets/shaders/4_Pipelines/solid.frag.spv");
		MaterialPtr	material3 = std::make_shared<Material>(shader3);
		material3->SetpolygonMode(VkPolygonMode::VK_POLYGON_MODE_LINE);
		
		std::vector<MaterialPtr> materials(4);
		materials[0] = material0;
		materials[1] = material1;
		materials[2] = material2;
		materials[3] = material3;
		// 加载模型
		std::vector<std::shared_ptr<Renderable>> renderables = OBJMeshParser::LoadFromFile("assets/models/suzanne.obj");
		
		for (int32 i = 0; i < materials.size(); ++i)
		{
			MeshPtr mesh = std::make_shared<Mesh>();
			for (int32 j = 0; j < renderables.size(); ++j)
			{
				mesh->AddSubMesh(renderables[j], materials[i]);
			}
			m_Meshes.push_back(mesh);
		}
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
	std::vector<MeshPtr>		  m_Meshes;
    VkSemaphore                   m_RenderComplete;
    std::vector<VulkanFence*>     m_Fences;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
    return new Pipelines(1120, 840, "Pipelines", cmdLine);
}
