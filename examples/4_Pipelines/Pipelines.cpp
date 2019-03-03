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
        m_VulkanRHI = GetVulkanRHI();
        m_Device = m_VulkanRHI->GetDevice()->GetInstanceHandle();
        
        m_MVPData.model.SetIdentity();
        m_MVPData.view.SetIdentity();
        m_MVPData.projection.SetIdentity();
        m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetWidth(), (float)GetHeight(), 0.01f, 3000.0f);
        
        LoadOBJ();
        LoadShader();
        CreateFences();
        CreateSemaphores();
        CreatePipelines();
        SetupCommandBuffers();
        
        m_Ready = true;
    }
    
    virtual void Exist() override
    {
        DestroyFences();
        DestorySemaphores();
        DestroyPipelines();

        m_IndexBuffer = nullptr;
        m_VertexBuffer = nullptr;
        m_Shader = nullptr;
    }
    
    virtual void Loop() override
    {
        if (!m_Ready)
        {
            return;
        }
        Draw();
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
        
        VkPipelineStageFlags waitStageMask = m_VulkanRHI->GetStageMask();
        std::shared_ptr<VulkanQueue> gfxQueue = m_VulkanRHI->GetDevice()->GetGraphicsQueue();
        std::shared_ptr<VulkanQueue> presentQueue = m_VulkanRHI->GetDevice()->GetPresentQueue();
        std::vector<VkCommandBuffer>& drawCmdBuffers = m_VulkanRHI->GetCommandBuffers();
        std::shared_ptr<VulkanSwapChain> swapChain = m_VulkanRHI->GetSwapChain();
        
        VkSemaphore presentCompleteSemaphore = VK_NULL_HANDLE;
        m_CurrentBackBuffer = swapChain->AcquireImageIndex(&presentCompleteSemaphore);
        
        VulkanFenceManager& fenceMgr = GetVulkanRHI()->GetDevice()->GetFenceManager();
        fenceMgr.WaitForFence(m_Fences[m_CurrentBackBuffer], MAX_uint64);
        fenceMgr.ResetFence(m_Fences[m_CurrentBackBuffer]);
        
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pWaitDstStageMask = &waitStageMask;
        submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_RenderComplete;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pCommandBuffers = &drawCmdBuffers[m_CurrentBackBuffer];
        submitInfo.commandBufferCount = 1;
        
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
        
        VkRenderPassBeginInfo renderPassBeginInfo;
        ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass = m_VulkanRHI->GetRenderPass();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = m_VulkanRHI->GetSwapChain()->GetWidth();
        renderPassBeginInfo.renderArea.extent.height = m_VulkanRHI->GetSwapChain()->GetHeight();
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;
        
        std::vector<VkCommandBuffer>& drawCmdBuffers = m_VulkanRHI->GetCommandBuffers();
        std::vector<VkFramebuffer> frameBuffers = m_VulkanRHI->GetFrameBuffers();
        for (int32 i = 0; i < drawCmdBuffers.size(); ++i)
        {
            renderPassBeginInfo.framebuffer = frameBuffers[i];
            
            VkViewport viewport = {};
            viewport.width = (float)renderPassBeginInfo.renderArea.extent.width;
            viewport.height = (float)renderPassBeginInfo.renderArea.extent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            
            VkRect2D scissor = {};
            scissor.extent.width = (uint32)viewport.width;
            scissor.extent.height = (uint32)viewport.height;
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            
            VkDeviceSize offsets[1] = { 0 };
            
            VERIFYVULKANRESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBeginInfo));
            vkCmdBeginRenderPass(drawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scissor);

			if (!m_IndexBuffer->Valid())
			{
				continue;
			}
			if (!m_VertexBuffer->Valid())
			{
				continue;
			}

            vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Shader->GetPipelineLayout(), 0, 1, &(m_Shader->GetDescriptorSet()), 0, nullptr);
            vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
            vkCmdBindVertexBuffers(drawCmdBuffers[i], 0, 1, m_VertexBuffer->GetVKBuffers().data(), offsets);
            vkCmdBindIndexBuffer(drawCmdBuffers[i], m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(drawCmdBuffers[i], m_IndexBuffer->GetIndexCount(), 1, 0, 0, 1);
            
			vkCmdEndRenderPass(drawCmdBuffers[i]);
            VERIFYVULKANRESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
        }
    }
    
    void CreatePipelines()
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
        ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
        inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        
        VkPipelineRasterizationStateCreateInfo rasterizationState;
        ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
        rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationState.cullMode = VK_CULL_MODE_NONE;
        rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationState.depthClampEnable = VK_FALSE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.depthBiasEnable = VK_FALSE;
        rasterizationState.lineWidth = 1.0f;
        
        VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
        blendAttachmentState[0].colorWriteMask = (
                                                  VK_COLOR_COMPONENT_R_BIT |
                                                  VK_COLOR_COMPONENT_G_BIT |
                                                  VK_COLOR_COMPONENT_B_BIT |
                                                  VK_COLOR_COMPONENT_A_BIT
                                                  );
        blendAttachmentState[0].blendEnable = VK_FALSE;
        
        VkPipelineColorBlendStateCreateInfo colorBlendState;
        ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
        colorBlendState.attachmentCount = 1;
        colorBlendState.pAttachments = blendAttachmentState;
        
        VkPipelineViewportStateCreateInfo viewportState;
        ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        
        std::vector<VkDynamicState> dynamicStateEnables;
        dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
        VkPipelineDynamicStateCreateInfo dynamicState;
        ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
        dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();
        dynamicState.pDynamicStates = dynamicStateEnables.data();
        
        VkPipelineDepthStencilStateCreateInfo depthStencilState;
        ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
        depthStencilState.depthTestEnable = VK_TRUE;
        depthStencilState.depthWriteEnable = VK_TRUE;
        depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
        depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
        depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilState.stencilTestEnable = VK_FALSE;
        depthStencilState.front = depthStencilState.back;
        
        VkPipelineMultisampleStateCreateInfo multisampleState;
        ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
        multisampleState.rasterizationSamples = m_VulkanRHI->GetSampleCount();
        multisampleState.pSampleMask = nullptr;
        
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
        ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
        ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = m_Shader->GetVertModule()->GetHandle();
        shaderStages[0].pName = "main";
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = m_Shader->GetFragModule()->GetHandle();
        shaderStages[1].pName = "main";
        
        // Material material(m_Shader);
        VkPipelineVertexInputStateCreateInfo vertexInputState = m_VertexBuffer->GetVertexInputStateInfo();
        
        VkGraphicsPipelineCreateInfo pipelineCreateInfo;
        ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
        pipelineCreateInfo.layout = m_Shader->GetPipelineLayout();
        pipelineCreateInfo.renderPass = m_VulkanRHI->GetRenderPass();
        pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
        pipelineCreateInfo.pStages = shaderStages.data();
        pipelineCreateInfo.pVertexInputState = &vertexInputState;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pDepthStencilState = &depthStencilState;
        pipelineCreateInfo.pDynamicState = &dynamicState;
        VERIFYVULKANRESULT(vkCreateGraphicsPipelines(m_Device, m_VulkanRHI->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));
    }
    
    void DestroyPipelines()
    {
        vkDestroyPipeline(m_Device, m_Pipeline, VULKAN_CPU_ALLOCATOR);
    }
    
    void UpdateUniformBuffers()
    {
        m_MVPData.view.SetIdentity();
        m_MVPData.view.SetOrigin(Vector4(0, 0, 30.0f));
        m_MVPData.view.SetInverse();
        
        m_Shader->SetUniformData("uboMVP", (uint8*)&m_MVPData, sizeof(UBOData));
    }
    
    void LoadShader()
    {
        m_Shader = Shader::Create("assets/shaders/4_Pipelines/pipelines.vert.spv", "assets/shaders/4_Pipelines/pipelines.frag.spv");
    }
    
    void LoadOBJ()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;
        tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, FileManager::GetFilePath("assets/models/suzanne.obj").c_str());
        
        std::vector<float> vertices;
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
        
        VertexStreamInfo streamInfo;
        streamInfo.size = vertices.size() * sizeof(float);
        streamInfo.channelMask = 1 << (int32)VertexAttribute::Position | 1 << (int32)VertexAttribute::Color;
        
        std::vector<VertexChannelInfo> channels(2);
        channels[0].attribute = VertexAttribute::Position;
        channels[0].format = VertexElementType::VET_Float3;
        channels[0].stream = 0;
        channels[0].offset = 0;
        channels[1].attribute = VertexAttribute::Color;
        channels[1].format = VertexElementType::VET_Float3;
        channels[1].stream = 0;
        channels[1].offset = 12;
        
		m_VertexBuffer = std::make_shared<VertexBuffer>();
        m_VertexBuffer->AddStream(streamInfo, channels, vertStreamData);
        
        // 索引数据
        uint32 indexStreamSize = indices.size() * sizeof(uint16);
        uint8* indexStreamData = new uint8[indexStreamSize];
        std::memcpy(indexStreamData, indices.data(), indexStreamSize);
        
		m_IndexBuffer = std::make_shared<IndexBuffer>(indexStreamData, indexStreamSize, PrimitiveType::PT_TriangleList, VkIndexType::VK_INDEX_TYPE_UINT16);
    }
    
    void CreateFences()
    {
        m_Fences.resize(GetVulkanRHI()->GetSwapChain()->GetBackBufferCount());
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
        vkCreateSemaphore(m_Device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
    }
    
    void DestorySemaphores()
    {
        vkDestroySemaphore(m_Device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
    }
    
    bool m_Ready;
    
    VkDevice m_Device;
    std::shared_ptr<VulkanRHI> m_VulkanRHI;
    
    std::shared_ptr<VertexBuffer> m_VertexBuffer;
	std::shared_ptr<IndexBuffer> m_IndexBuffer;
    UBOData m_MVPData;
    std::shared_ptr<Shader> m_Shader;
    
    VkSemaphore m_RenderComplete;
    std::vector<VulkanFence*> m_Fences;
    VkPipeline m_Pipeline;
    
    uint32 m_CurrentBackBuffer;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
    return new Pipelines(800, 600, "Pipelines", cmdLine);
}
