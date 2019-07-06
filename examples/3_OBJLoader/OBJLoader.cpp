#include "Common/Common.h"
#include "Common/Log.h"
#include "Configuration/Platform.h"
#include "Application/AppModeBase.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanQueue.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanMemory.h"
#include "Vulkan/VulkanCommandBuffer.h"
#include "Vulkan/VulkanContext.h"
#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"
#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Data/IndexBuffer.h"
#include "Graphics/Data/VulkanBuffer.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Renderer/Renderable.h"
#include "Graphics/Command/DrawCommand.h"
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
		, m_ImageIndex(0)
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
		InitShaderParams();
		LoadAssets();
		RecordCommandBuffers();
		m_Ready = true;
	}

	virtual void Exist() override
	{
		WaitFences(m_ImageIndex);
		Release();
        DestroyAssets();
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
		UpdateShaderParams();

		m_ImageIndex = AcquireImageIndex();
		
        std::shared_ptr<VulkanSwapChain> swapChain   = GetVulkanRHI()->GetSwapChain();
        VkPipelineStageFlags waitStageMask           = GetVulkanRHI()->GetStageMask();
        std::shared_ptr<VulkanQueue> gfxQueue        = GetVulkanRHI()->GetDevice()->GetGraphicsQueue();
        std::shared_ptr<VulkanQueue> presentQueue    = GetVulkanRHI()->GetDevice()->GetPresentQueue();
        
        swapChain->Present(gfxQueue, presentQueue, &m_RenderComplete);
	}
	
	void RecordCommandBuffers()
	{
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
        
		VulkanCommandBufferManager* commandBufferManager = GetVulkanRHI()->GetDevice()->GetImmediateContext().GetCommandBufferManager();
        VulkanCommandListContextImmediate& cmdContext = GetVulkanRHI()->GetDevice()->GetImmediateContext();
        
        VulkanCmdBuffer* cmdBuffer  = commandBufferManager->GetActiveCmdBuffer();

        VkCommandBuffer vkCmdBuffer = cmdBuffer->GetHandle();
        
        renderPassBeginInfo.framebuffer = m_FrameBuffers[m_ImageIndex];
        vkCmdBeginRenderPass(vkCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdSetViewport(vkCmdBuffer, 0, 1, &viewport);
        vkCmdSetScissor(vkCmdBuffer, 0, 1, &scissor);
        
        m_DrawCommand->Prepare(cmdBuffer, &cmdContext);
        
        vkCmdEndRenderPass(vkCmdBuffer);
        commandBufferManager->SubmitActiveCmdBuffer();
	}
    
	void UpdateShaderParams()
	{
        float deltaTime = Engine::Get()->GetDeltaTime();
        m_MVPData.model.AppendRotation(90.0f * deltaTime, Vector3::UpVector);
	}
    
	void InitShaderParams()
	{
        m_MVPData.model.SetIdentity();

		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector4(0, 0, -30.0f));
		m_MVPData.view.SetInverse();
        
		m_MVPData.projection.SetIdentity();
        m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetFrameWidth(), (float)GetFrameHeight(), 0.01f, 3000.0f);
	}

    void DestroyAssets()
    {
        m_Shader       = nullptr;
        m_Material     = nullptr;
        m_Renderable   = nullptr;
        m_DrawCommand  = nullptr;
    }
    
	void LoadAssets()
	{
        m_Renderable  = MeshLoader::LoadFromFile("assets/models/suzanne.obj")[0];
        m_Shader      = Shader::Create("assets/shaders/3_OBJLoader/obj.vert.spv", "assets/shaders/3_OBJLoader/obj.frag.spv");
        m_Material    = std::make_shared<Material>(m_Shader);
		m_Material->SetParam("uboMVP", &m_MVPData, sizeof(m_MVPData));
        
        m_DrawCommand = std::make_shared<MeshDrawCommand>();
        m_DrawCommand->material   = m_Material;
        m_DrawCommand->renderable = m_Renderable;
        
        MLOG("DrawCommand Prepare done.")
	}
    
private:
	
	bool 							    m_Ready;
    
    std::shared_ptr<MeshDrawCommand>    m_DrawCommand;
    
    std::shared_ptr<Shader>             m_Shader;
	std::shared_ptr<Material>		    m_Material;
	std::shared_ptr<Renderable>         m_Renderable;

	UBOData 						    m_MVPData;
    
	uint32 							    m_ImageIndex;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return new OBJLoaderMode(800, 600, "OBJLoader", cmdLine);
}
