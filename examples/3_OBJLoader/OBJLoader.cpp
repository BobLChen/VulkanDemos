#include "Common/Common.h"
#include "Common/Log.h"

#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanQueue.h"
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan/VulkanMemory.h"
#include "Vulkan/VulkanCommandBuffer.h"
#include "Vulkan/VulkanContext.h"
#include "Vulkan/VulkanFence.h"
#include "Vulkan/VulkanCommandBuffer.h"

#include "Graphics/Data/VertexBuffer.h"
#include "Graphics/Data/IndexBuffer.h"
#include "Graphics/Data/VulkanBuffer.h"
#include "Graphics/Shader/Shader.h"
#include "Graphics/Material/Material.h"
#include "Graphics/Renderer/Renderable.h"
#include "Graphics/Command/DrawCommand.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "File/FileManager.h"
#include "Loader/MeshLoader.h"
#include "Application/AppModuleBase.h"

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
		InitFences();
		InitShaderParams();
		LoadAssets();
		m_Ready = true;
		return true;
	}

	virtual void Exist() override
	{
		Release();
		DestroyFences();
        DestroyAssets();
	}
    
	virtual void Loop(float time, float delta) override
	{
		if (m_Ready) {
			Draw(time, delta);
		}
	}

private:
    
	struct UBOData
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	void Draw(float time, float delta)
	{
		UpdateShaderParams(time, delta);

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
		int32 frameWidth  = GetVulkanRHI()->GetSwapChain()->GetWidth();
		int32 frameHeight = GetVulkanRHI()->GetSwapChain()->GetHeight();
		
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
		renderPassBeginInfo.renderArea.extent.width  = frameWidth;
		renderPassBeginInfo.renderArea.extent.height = frameHeight;
		renderPassBeginInfo.framebuffer = m_FrameBuffers[m_ImageIndex];;

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = frameHeight;
		viewport.width  =  (float)frameWidth;
		viewport.height = -(float)frameHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = (uint32)frameWidth;
		scissor.extent.height = (uint32)frameHeight;
		scissor.offset.x      = 0;
		scissor.offset.y      = 0;
        
        VkCommandBuffer vkCmdBuffer = cmdBuffer->GetHandle();
        
        vkCmdBeginRenderPass(vkCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdSetViewport(vkCmdBuffer, 0, 1, &viewport);
        vkCmdSetScissor(vkCmdBuffer, 0, 1, &scissor);

		for (int32 i = 0; i < m_DrawCommands.size(); ++i) {
			m_DrawCommands[i]->Prepare(cmdBuffer, &cmdContext);
		}
        
        vkCmdEndRenderPass(vkCmdBuffer);
	}
    
	void UpdateShaderParams(float time, float delta)
	{
		for (int32 i = 0; i < m_MVPDatas.size(); ++i)
		{
			Vector3 position = m_MVPDatas[i].model.GetOrigin();

			m_MVPDatas[i].model.AppendRotation(90.0f * delta, Vector3::UpVector, &position);
			m_Materials[i]->SetParam("uboMVP", &(m_MVPDatas[i]), sizeof(UBOData));
		}
	}
    
	void InitShaderParams()
	{
		m_MVPDatas.resize(25 * 25);
        
		float fw = GetVulkanRHI()->GetSwapChain()->GetWidth();
		float fh = GetVulkanRHI()->GetSwapChain()->GetHeight();

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
				m_MVPDatas[index].projection.Perspective(MMath::DegreesToRadians(60.0f), fw, fh, 0.01f, 3000.0f);
			}
		}

	}

	void DestroyFences()
	{
		VkDevice vkDevice = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		for (int32 i = 0; i < m_RenderingDoneSemaphores.size(); ++i) {
			vkDestroySemaphore(vkDevice, m_RenderingDoneSemaphores[i], VULKAN_CPU_ALLOCATOR);
		}
		m_RenderingDoneSemaphores.clear();
	}

	void InitFences()
	{
		int32 bufferCount = GetVulkanRHI()->GetSwapChain()->GetBackBufferCount();
		VkDevice vkDevice = GetVulkanRHI()->GetDevice()->GetInstanceHandle();

		for (int32 i = 0; i < bufferCount; ++i)
		{
			VkSemaphore semaphore;
			VkSemaphoreCreateInfo createInfo;
			ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
			vkCreateSemaphore(vkDevice, &createInfo, VULKAN_CPU_ALLOCATOR, &semaphore);
			m_RenderingDoneSemaphores.push_back(semaphore);
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
