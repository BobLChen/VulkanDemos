#include "Common/Common.h"
#include "Common/Log.h"
#include "Configuration/Platform.h"
#include "Application/AppModuleBase.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanQueue.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanMemory.h"
#include "Vulkan/VulkanCommandBuffer.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanFence.h"
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

class OBJLoaderMode : public AppModuleBase
{
public:
	OBJLoaderMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModuleBase(width, height, title)
		, m_Ready(false)
		, m_ImageIndex(0)
	{
        
	}

	virtual ~OBJLoaderMode()
	{

	}

	virtual bool PreInit() override
	{
		return true;
	}

	virtual bool Init() override
	{
		Prepare();

		for (int32 i = 0; i < GetFrameCount(); ++i)
		{
			VkSemaphore semaphore;
			VkSemaphoreCreateInfo createInfo;
			ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
			vkCreateSemaphore(GetDevice(), &createInfo, VULKAN_CPU_ALLOCATOR, &semaphore);
			m_RenderingDoneSemaphores.push_back(semaphore);
		}

		InitShaderParams();
		LoadAssets();
		m_Ready = true;

		return true;
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

		m_ImageIndex = GetVulkanRHI()->GetSwapChain()->AcquireImageIndex(&m_AcquiredSemaphore);

		VulkanCommandListContextImmediate& cmdContext    = GetVulkanRHI()->GetDevice()->GetImmediateContext();
		VulkanCommandBufferManager* commandBufferManager = cmdContext.GetCommandBufferManager();
        VulkanCmdBuffer* cmdBuffer  = commandBufferManager->GetActiveCmdBuffer();

		std::shared_ptr<VulkanSwapChain> swapChain   = GetVulkanRHI()->GetSwapChain();
        std::shared_ptr<VulkanQueue> gfxQueue        = GetVulkanRHI()->GetDevice()->GetGraphicsQueue();
        std::shared_ptr<VulkanQueue> presentQueue    = GetVulkanRHI()->GetDevice()->GetPresentQueue();

		RecordCommandBuffers(cmdContext, cmdBuffer);

		cmdBuffer->End();
		cmdBuffer->AddWaitSemaphore(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, m_AcquiredSemaphore);

		presentQueue->Submit(cmdBuffer, m_RenderingDoneSemaphores[m_ImageIndex]);
		
        swapChain->Present(gfxQueue, presentQueue, &(m_RenderingDoneSemaphores[m_ImageIndex]));

		commandBufferManager->NewActiveCommandBuffer();
	}
	
	void RecordCommandBuffers(VulkanCommandListContextImmediate& cmdContext, VulkanCmdBuffer* cmdBuffer)
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
        
        VkCommandBuffer vkCmdBuffer = cmdBuffer->GetHandle();
        
        renderPassBeginInfo.framebuffer = m_FrameBuffers[m_ImageIndex];
        vkCmdBeginRenderPass(vkCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdSetViewport(vkCmdBuffer, 0, 1, &viewport);
        vkCmdSetScissor(vkCmdBuffer, 0, 1, &scissor);

		for (int32 i = 0; i < m_DrawCommands.size(); ++i)
		{
			m_DrawCommands[i]->Prepare(cmdBuffer, &cmdContext);
		}
        
        vkCmdEndRenderPass(vkCmdBuffer);
	}
    
	void UpdateShaderParams()
	{
        float deltaTime = Engine::Get()->GetDeltaTime();
        
		for (int32 i = 0; i < m_MVPDatas.size(); ++i)
		{
			Vector3 position = m_MVPDatas[i].model.GetOrigin();

			m_MVPDatas[i].model.AppendRotation(90.0f * deltaTime, Vector3::UpVector, &position);
			m_Materials[i]->SetParam("uboMVP", &(m_MVPDatas[i]), sizeof(UBOData));
		}
	}
    
	void InitShaderParams()
	{
		m_MVPDatas.resize(25 * 25);

		float tx = 0;
		float ty = 0;

		for (int32 row = 0; row < 25; ++row)
		{
			for (int32 col = 0; col < 25; ++col)
			{
				Vector3 position(0, 0, 0);
				position.x = (row - 12.5f) * 25;
				position.y = (col - 12.5f) * 25;

				int index = row * 25 + col;

				m_MVPDatas[index].model.SetIdentity();
				m_MVPDatas[index].model.AppendTranslation(position);

				m_MVPDatas[index].view.SetIdentity();
				m_MVPDatas[index].view.SetOrigin(Vector4(0, 0, -500.0f));
				m_MVPDatas[index].view.SetInverse();
        
				m_MVPDatas[index].projection.SetIdentity();
				m_MVPDatas[index].projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetFrameWidth(), (float)GetFrameHeight(), 0.01f, 3000.0f);
			}
		}
	}

    void DestroyAssets()
    {
        m_Shader       = nullptr;
        m_Renderable   = nullptr;
		m_Materials.clear();
		m_DrawCommands.clear();
    }
    
	void LoadAssets()
	{
        m_Renderable  = MeshLoader::LoadFromFile("assets/models/suzanne.obj")[0];
        m_Shader      = Shader::Create("assets/shaders/3_OBJLoader/obj.vert.spv", "assets/shaders/3_OBJLoader/obj.frag.spv");

		m_Materials.resize(m_MVPDatas.size());
		m_DrawCommands.resize(m_MVPDatas.size());
		for (int32 i = 0; i < m_MVPDatas.size(); ++i)
		{
			m_Materials[i] = std::make_shared<Material>(m_Shader);
			m_Materials[i]->SetParam("uboMVP", &(m_MVPDatas[i]), sizeof(UBOData));

			m_DrawCommands[i] = std::make_shared<MeshDrawCommand>();
			m_DrawCommands[i]->material   = m_Materials[i];
			m_DrawCommands[i]->renderable = m_Renderable;
		}

        MLOG("DrawCommand Prepare done.")
	}
    
private:

	typedef std::vector<std::shared_ptr<MeshDrawCommand>>	MeshDrawCommandList;
	typedef std::vector<UBOData>							MeshUBODataList;
	typedef std::vector<std::shared_ptr<Material>>			MaterialList;

	bool 							    m_Ready;
    
    MeshDrawCommandList					m_DrawCommands;
    MeshUBODataList						m_MVPDatas;

    std::shared_ptr<Shader>             m_Shader;
	std::shared_ptr<Renderable>         m_Renderable;
	MaterialList						m_Materials;

	std::vector<VkSemaphore>			m_RenderingDoneSemaphores;
	VkSemaphore							m_AcquiredSemaphore;

	uint32 							    m_ImageIndex;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<OBJLoaderMode>(800, 600, "OBJLoader", cmdLine);
}
