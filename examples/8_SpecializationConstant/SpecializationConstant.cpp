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
#include "Utils/Alignment.h"
#include <vector>

class SpeciallizationConstantMode : public AppModeBase
{
public:
    SpeciallizationConstantMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
    : AppModeBase(width, height, title)
    , m_Ready(false)
    , m_ImageIndex(0)
    {
        
    }
    
    virtual ~SpeciallizationConstantMode()
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
        CreatePipeline();
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
        DestroyPipeline();
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
    
    struct Pipelines
    {
        VkPipeline phong;
        VkPipeline rim;
        VkPipeline texture;
    };
    
    void Draw()
    {
        UpdateUniformBuffers();
        m_ImageIndex = AcquireImageIndex();
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
        renderPassBeginInfo.renderPass = m_RenderPass;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
        renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;
        
        VkDeviceSize offsets[1] = { 0 };
        
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = m_FrameHeight;
        viewport.width  =  (float)(m_FrameWidth / 3.0f);
        viewport.height = -(float)m_FrameHeight;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        
        VkRect2D scissor = {};
        scissor.extent.width  = (uint32)(m_FrameWidth / 3);
        scissor.extent.height = (uint32)m_FrameHeight;
        scissor.offset.x      = 0;
        scissor.offset.y      = 0;
        
        for (int32 i = 0; i < m_DrawCmdBuffers.size(); ++i)
        {
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];
            VERIFYVULKANRESULT(vkBeginCommandBuffer(m_DrawCmdBuffers[i], &cmdBeginInfo));
            vkCmdBeginRenderPass(m_DrawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindDescriptorSets(m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Shader->GetPipelineLayout(), 0, 1, &(m_DescriptorSets[i]), 0, nullptr);
            vkCmdBindVertexBuffers(m_DrawCmdBuffers[i], 0, 1, m_Renderable->GetVertexBuffer()->GetVKBuffers().data(), offsets);
            vkCmdBindIndexBuffer(m_DrawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetBuffer(), 0, m_Renderable->GetIndexBuffer()->GetIndexType());
            
            viewport.x = 0;
            scissor.offset.x = 0;
            vkCmdSetViewport(m_DrawCmdBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_DrawCmdBuffers[i], 0, 1, &scissor);
            vkCmdBindPipeline(m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.phong);
            vkCmdDrawIndexed(m_DrawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
            
            viewport.x += viewport.width;
            scissor.offset.x += viewport.width;
            vkCmdSetViewport(m_DrawCmdBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_DrawCmdBuffers[i], 0, 1, &scissor);
            vkCmdBindPipeline(m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.rim);
            vkCmdDrawIndexed(m_DrawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
            
            viewport.x += viewport.width;
            scissor.offset.x += viewport.width;
            vkCmdSetViewport(m_DrawCmdBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_DrawCmdBuffers[i], 0, 1, &scissor);
            vkCmdBindPipeline(m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.texture);
            vkCmdDrawIndexed(m_DrawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
            
            vkCmdEndRenderPass(m_DrawCmdBuffers[i]);
            VERIFYVULKANRESULT(vkEndCommandBuffer(m_DrawCmdBuffers[i]));
        }
    }
    
    void DestroyPipeline()
    {
        vkDestroyPipeline(GetDevice(), m_Pipelines.phong,   VULKAN_CPU_ALLOCATOR);
        vkDestroyPipeline(GetDevice(), m_Pipelines.rim,     VULKAN_CPU_ALLOCATOR);
        vkDestroyPipeline(GetDevice(), m_Pipelines.texture, VULKAN_CPU_ALLOCATOR);
    }
    
    void CreatePipeline()
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
        ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
        inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        
        VkPipelineRasterizationStateCreateInfo rasterizationState;
        ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
        rasterizationState.polygonMode                = VK_POLYGON_MODE_FILL;
        rasterizationState.cullMode                = VK_CULL_MODE_NONE;
        rasterizationState.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationState.depthClampEnable        = VK_FALSE;
        rasterizationState.rasterizerDiscardEnable = VK_FALSE;
        rasterizationState.depthBiasEnable         = VK_FALSE;
        rasterizationState.lineWidth                = 1.0f;
        
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
        colorBlendState.pAttachments    = blendAttachmentState;
        
        VkPipelineViewportStateCreateInfo viewportState;
        ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
        viewportState.viewportCount = 1;
        viewportState.scissorCount  = 1;
        
        std::vector<VkDynamicState> dynamicStateEnables;
        dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
        
        VkPipelineDynamicStateCreateInfo dynamicState;
        ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
        dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();
        dynamicState.pDynamicStates    = dynamicStateEnables.data();
        
        VkPipelineDepthStencilStateCreateInfo depthStencilState;
        ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
        depthStencilState.depthTestEnable       = VK_TRUE;
        depthStencilState.depthWriteEnable      = VK_TRUE;
        depthStencilState.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilState.depthBoundsTestEnable = VK_FALSE;
        depthStencilState.back.failOp           = VK_STENCIL_OP_KEEP;
        depthStencilState.back.passOp           = VK_STENCIL_OP_KEEP;
        depthStencilState.back.compareOp        = VK_COMPARE_OP_ALWAYS;
        depthStencilState.stencilTestEnable     = VK_FALSE;
        depthStencilState.front                 = depthStencilState.back;
        
        VkPipelineMultisampleStateCreateInfo multisampleState;
        ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
        multisampleState.rasterizationSamples = GetVulkanRHI()->GetSampleCount();
        multisampleState.pSampleMask          = nullptr;
        
        const VertexInputDeclareInfo& inputDeclareInfo = m_Renderable->GetVertexBuffer()->GetVertexInputStateInfo();
        const std::vector<VertexInputDeclareInfo::BindingDescription>& inVertexBindings = inputDeclareInfo.GetBindings();
        std::vector<VkVertexInputBindingDescription> vertexInputBindings(inVertexBindings.size());
        for (int32 i = 0; i < vertexInputBindings.size(); ++i)
        {
            vertexInputBindings[i].binding   = inVertexBindings[i].binding;
            vertexInputBindings[i].stride    = inVertexBindings[i].stride;
            vertexInputBindings[i].inputRate = inVertexBindings[i].inputRate;
        }
        
        const std::vector<VertexInputDeclareInfo::AttributeDescription>& inVertexDescInfos = inputDeclareInfo.GetAttributes();
        std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(inVertexDescInfos.size());
        for (int32 i = 0; i < vertexInputAttributs.size(); ++i)
        {
            vertexInputAttributs[i].binding  = inVertexDescInfos[i].binding;
            vertexInputAttributs[i].location = m_Shader->GetVertexInputBindingInfo().GetLocation(inVertexDescInfos[i].attribute);
            vertexInputAttributs[i].format   = inVertexDescInfos[i].format;
            vertexInputAttributs[i].offset   = inVertexDescInfos[i].offset;
        }
        
        VkPipelineVertexInputStateCreateInfo vertexInputState;
        ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
        vertexInputState.vertexBindingDescriptionCount   = (uint32_t)vertexInputBindings.size();
        vertexInputState.pVertexBindingDescriptions      = vertexInputBindings.data();
        vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributs.size();
        vertexInputState.pVertexAttributeDescriptions    = vertexInputAttributs.data();
        
        struct SpecializationData
        {
            uint32 lightingModel;
            float intensity = 1.0f;
        } specializationData;
        
        std::vector<VkSpecializationMapEntry> specializationMapEntries(2);
        specializationMapEntries[0].constantID = 0;
        specializationMapEntries[0].size       = sizeof(uint32);
        specializationMapEntries[0].offset     = 0;
        specializationMapEntries[1].constantID = 1;
        specializationMapEntries[1].size       = sizeof(float);
        specializationMapEntries[1].offset     = specializationMapEntries[0].size;
        
        VkSpecializationInfo specializationInfo = {};
        specializationInfo.dataSize      = sizeof(SpecializationData);
        specializationInfo.mapEntryCount = static_cast<uint32_t>(specializationMapEntries.size());
        specializationInfo.pMapEntries   = specializationMapEntries.data();
        specializationInfo.pData         = &specializationData;
        
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
        ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
        ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
        shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = m_Shader->GetVertModule()->GetHandle();
        shaderStages[0].pName  = "main";
        shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = m_Shader->GetFragModule()->GetHandle();
        shaderStages[1].pName  = "main";
        shaderStages[1].pSpecializationInfo = &specializationInfo;
        
        VkGraphicsPipelineCreateInfo pipelineCreateInfo;
        ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
        pipelineCreateInfo.layout                 = m_Shader->GetPipelineLayout();
        pipelineCreateInfo.renderPass             = m_RenderPass;
        pipelineCreateInfo.stageCount             = (uint32_t)shaderStages.size();
        pipelineCreateInfo.pStages                = shaderStages.data();
        pipelineCreateInfo.pVertexInputState      = &vertexInputState;
        pipelineCreateInfo.pInputAssemblyState    = &inputAssemblyState;
        pipelineCreateInfo.pRasterizationState    = &rasterizationState;
        pipelineCreateInfo.pColorBlendState       = &colorBlendState;
        pipelineCreateInfo.pMultisampleState      = &multisampleState;
        pipelineCreateInfo.pViewportState         = &viewportState;
        pipelineCreateInfo.pDepthStencilState     = &depthStencilState;
        pipelineCreateInfo.pDynamicState          = &dynamicState;
        
        specializationData.lightingModel = 0;
        specializationData.intensity     = 1.0f;
        VERIFYVULKANRESULT(vkCreateGraphicsPipelines(GetDevice(), GetVulkanRHI()->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipelines.phong));
        
        specializationData.lightingModel = 1;
        specializationData.intensity     = 0.7f;
        VERIFYVULKANRESULT(vkCreateGraphicsPipelines(GetDevice(), GetVulkanRHI()->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipelines.rim));
        
        specializationData.lightingModel = 2;
        specializationData.intensity     = 1.5f;
        VERIFYVULKANRESULT(vkCreateGraphicsPipelines(GetDevice(), GetVulkanRHI()->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipelines.texture));
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
            writeDescriptorSet.dstSet          = m_DescriptorSets[i];
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSet.pBufferInfo     = &(m_MVPBuffers[i]->GetDescriptorBufferInfo());
            writeDescriptorSet.dstBinding      = 0;
            vkUpdateDescriptorSets(GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
            
            writeDescriptorSet.dstSet          = m_DescriptorSets[i];
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptorSet.pImageInfo      = &(m_Diffuse->GetDescriptorImageInfo());
            writeDescriptorSet.dstBinding      = 1;
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
        descriptorPoolInfo.maxSets       = GetFrameCount();
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
        m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetFrameWidth()
                                          / 3.0f, (float)GetFrameHeight(), 0.01f, 3000.0f);
        
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
        m_Shader   = Shader::Create("assets/shaders/8_SpecializationConstant/obj.vert.spv", "assets/shaders/8_SpecializationConstant/obj.frag.spv");
        m_Material = std::make_shared<Material>(m_Shader);
        
        m_Diffuse  = std::make_shared<Texture2D>();
		m_Diffuse->LoadFromFile("assets/textures/head_diffuse.jpg");

        m_Renderable = MeshLoader::LoadFromFile("assets/models/head.obj")[0];
    }
    
private:
    bool                            m_Ready;
    
    std::shared_ptr<Shader>         m_Shader;
    std::shared_ptr<Material>       m_Material;
    std::shared_ptr<Renderable>     m_Renderable;
    
    std::shared_ptr<Texture2D>		m_Diffuse;

    UBOData 						m_MVPData;
    std::vector<VulkanBuffer*>      m_MVPBuffers;
    
    std::vector<VkDescriptorSet>    m_DescriptorSets;
    VkDescriptorPool                m_DescriptorPool;
    Pipelines                       m_Pipelines;
    
    uint32                          m_ImageIndex;
};

AppModeBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
    return new SpeciallizationConstantMode(1120, 840, "SpeciallizationConstantMode", cmdLine);
}
