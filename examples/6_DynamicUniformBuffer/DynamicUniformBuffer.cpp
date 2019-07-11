#include "Common/Common.h"
#include "Common/Log.h"
#include "Configuration/Platform.h"
#include "Application/AppModuleBase.h"
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

#define CUBE_SIZE  5
#define CUBE_COUNT CUBE_SIZE * CUBE_SIZE * CUBE_SIZE
#define MATERIAL_COUT 2

class DynamicUniformBufferMode : public AppModuleBase
{
public:
    DynamicUniformBufferMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
    : AppModuleBase(width, height, title)
    , m_Ready(false)
    , m_ImageIndex(0)
    {
        
    }
    
    virtual ~DynamicUniformBufferMode()
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
		CreateDescriptorLayout();
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
		DestroyDescriptorLayout();
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
    
    struct ModelUBOData
    {
        Matrix4x4 model;
    };

    struct MaterialUBOData
    {
        Vector4 color;
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
        
        uint32 bufferAlign = GetVulkanRHI()->GetDevice()->GetLimits().minUniformBufferOffsetAlignment;
        uint32 modelAlign  = Align(sizeof(ModelUBOData), bufferAlign);
        uint32 vpAlign     = Align(sizeof(MaterialUBOData), bufferAlign);

        for (int32 i = 0; i < m_DrawCmdBuffers.size(); ++i)
        {
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];

            VERIFYVULKANRESULT(vkBeginCommandBuffer(m_DrawCmdBuffers[i], &cmdBeginInfo));
            vkCmdBeginRenderPass(m_DrawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdSetViewport(m_DrawCmdBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_DrawCmdBuffers[i], 0, 1, &scissor);
            vkCmdBindPipeline(m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
            vkCmdBindVertexBuffers(m_DrawCmdBuffers[i], 0, 1, m_Renderable->GetVertexBuffer()->GetVKBuffers().data(), offsets);
            vkCmdBindIndexBuffer(m_DrawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetBuffer(), 0, m_Renderable->GetIndexBuffer()->GetIndexType());
            
			for (int32 j = 0; j < CUBE_COUNT; ++j)
			{
                std::vector<uint32> dynamicOffsets(2);
                dynamicOffsets[0] = j * modelAlign;
                if (j < CUBE_COUNT / 2) 
                {
                    dynamicOffsets[1] = 0;
                }
                else
                {
                    dynamicOffsets[1] = vpAlign * 1;
                }
				vkCmdBindDescriptorSets(m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &(m_DescriptorSets[i]), dynamicOffsets.size(), dynamicOffsets.data());
				vkCmdDrawIndexed(m_DrawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
			}

			vkCmdEndRenderPass(m_DrawCmdBuffers[i]);
            VERIFYVULKANRESULT(vkEndCommandBuffer(m_DrawCmdBuffers[i]));
        }
    }

	void DestroyPipeline()
	{
		vkDestroyPipeline(GetDevice(), m_Pipeline, VULKAN_CPU_ALLOCATOR);
	}

	void CreatePipeline()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		ZeroVulkanStruct(inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
		inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
		VkPipelineRasterizationStateCreateInfo rasterizationState;
		ZeroVulkanStruct(rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
		rasterizationState.polygonMode 			   = VK_POLYGON_MODE_FILL;
		rasterizationState.cullMode                = VK_CULL_MODE_NONE;
		rasterizationState.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationState.depthClampEnable        = VK_FALSE;
		rasterizationState.rasterizerDiscardEnable = VK_FALSE;
		rasterizationState.depthBiasEnable         = VK_FALSE;
		rasterizationState.lineWidth 			   = 1.0f;
        
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
		depthStencilState.depthTestEnable 		= VK_TRUE;
		depthStencilState.depthWriteEnable 		= VK_TRUE;
		depthStencilState.depthCompareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilState.depthBoundsTestEnable = VK_FALSE;
		depthStencilState.back.failOp 			= VK_STENCIL_OP_KEEP;
		depthStencilState.back.passOp 			= VK_STENCIL_OP_KEEP;
		depthStencilState.back.compareOp 		= VK_COMPARE_OP_ALWAYS;
		depthStencilState.stencilTestEnable 	= VK_FALSE;
		depthStencilState.front 				= depthStencilState.back;

		VkPipelineMultisampleStateCreateInfo multisampleState;
		ZeroVulkanStruct(multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
		multisampleState.rasterizationSamples = GetVulkanRHI()->GetSampleCount();
		multisampleState.pSampleMask 		  = nullptr;
		
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

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
		ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = m_Shader->GetVertModule()->GetHandle();
		shaderStages[0].pName  = "main";
		shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = m_Shader->GetFragModule()->GetHandle();
		shaderStages[1].pName  = "main";

		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout 				= m_PipelineLayout;
		pipelineCreateInfo.renderPass 			= m_RenderPass;
		pipelineCreateInfo.stageCount 			= (uint32_t)shaderStages.size();
		pipelineCreateInfo.pStages 				= shaderStages.data();
		pipelineCreateInfo.pVertexInputState 	= &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState 	= &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState 	= &rasterizationState;
		pipelineCreateInfo.pColorBlendState 	= &colorBlendState;
		pipelineCreateInfo.pMultisampleState 	= &multisampleState;
		pipelineCreateInfo.pViewportState 		= &viewportState;
		pipelineCreateInfo.pDepthStencilState 	= &depthStencilState;
		pipelineCreateInfo.pDynamicState 		= &dynamicState;
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(GetDevice(), GetVulkanRHI()->GetPipelineCache(), 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));
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
            allocInfo.pSetLayouts        = &m_DescriptorSetLayout;
            VERIFYVULKANRESULT(vkAllocateDescriptorSets(GetDevice(), &allocInfo, &(m_DescriptorSets[i])));
            
            VkWriteDescriptorSet writeDescriptorSet0;
            ZeroVulkanStruct(writeDescriptorSet0, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
            writeDescriptorSet0.dstSet          = m_DescriptorSets[i];
            writeDescriptorSet0.descriptorCount = 1;
            writeDescriptorSet0.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            writeDescriptorSet0.pBufferInfo     = &(m_ModelBuffers[i]->GetDescriptorBufferInfo());
            writeDescriptorSet0.dstBinding      = 0;
            vkUpdateDescriptorSets(GetDevice(), 1, &writeDescriptorSet0, 0, nullptr);

            VkWriteDescriptorSet writeDescriptorSet1;
            ZeroVulkanStruct(writeDescriptorSet1, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
            writeDescriptorSet1.dstSet          = m_DescriptorSets[i];
            writeDescriptorSet1.descriptorCount = 1;
            writeDescriptorSet1.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            writeDescriptorSet1.pBufferInfo     = &(m_ViewProjectionBuffers[i]->GetDescriptorBufferInfo());
            writeDescriptorSet1.dstBinding      = 1;
            vkUpdateDescriptorSets(GetDevice(), 1, &writeDescriptorSet1, 0, nullptr);
        }
    }

	void DestroyDescriptorLayout()
	{
		vkDestroyPipelineLayout(GetDevice(), m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyDescriptorSetLayout(GetDevice(), m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
	}

	void CreateDescriptorLayout()
	{
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;
        setLayoutBindings.resize(2);

		setLayoutBindings[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		setLayoutBindings[0].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
		setLayoutBindings[0].binding         = 0;
		setLayoutBindings[0].descriptorCount = 1;

        setLayoutBindings[1].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		setLayoutBindings[1].stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;
		setLayoutBindings[1].binding         = 1;
		setLayoutBindings[1].descriptorCount = 1;

		VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo;
		ZeroVulkanStruct(setLayoutCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		setLayoutCreateInfo.pBindings    = setLayoutBindings.data();
		setLayoutCreateInfo.bindingCount = setLayoutBindings.size();
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(GetDevice(), &setLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));
		
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		ZeroVulkanStruct(pipelineLayoutCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts    = &m_DescriptorSetLayout;

		VERIFYVULKANRESULT(vkCreatePipelineLayout(GetDevice(), &pipelineLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
	}
    
    void CreateDescriptorPool()
    {
		VkDescriptorPoolSize poolSize;
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSize.descriptorCount = 2;

        VkDescriptorPoolCreateInfo descriptorPoolInfo;
        ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
        descriptorPoolInfo.poolSizeCount = 1;
        descriptorPoolInfo.pPoolSizes    = &poolSize;
        descriptorPoolInfo.maxSets       = GetFrameCount();
        VERIFYVULKANRESULT(vkCreateDescriptorPool(GetDevice(), &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
    }
    
    void DestroyDescriptorPool()
    {
        vkDestroyDescriptorPool(GetDevice(), m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
    }
    
    void CreateUniformBuffers()
    {
        uint32 bufferAlignment = GetVulkanRHI()->GetDevice()->GetLimits().minUniformBufferOffsetAlignment;

        // model matrix dynamic uniform buffer
		uint32 modelAlign = Align(sizeof(ModelUBOData), bufferAlignment);
		uint32 modelSize  = modelAlign * CUBE_COUNT;
        m_ModelBuffers.resize(GetFrameCount());
        for (int32 i = 0; i < GetFrameCount(); ++i) 
		{
            m_ModelBuffers[i] = VulkanBuffer::CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, modelSize);
        }

        m_ModelUBODataDatas.resize(modelSize);
        for (int32 i = 0; i < CUBE_COUNT; ++i)
		{
			uint8* dataPtr = m_ModelUBODataDatas.data() + i * modelAlign;
			ModelUBOData* modelMatrix = (ModelUBOData*)dataPtr;
            
            modelMatrix->model.SetIdentity();
            
			dataPtr += modelAlign;
		}

        // material dynamic uniform buffer
        // simulate two material
        uint32 vpAlign    = Align(sizeof(MaterialUBOData), bufferAlignment);
        uint32 vpSize     = vpAlign * MATERIAL_COUT;
        m_ViewProjectionBuffers.resize(GetFrameCount());
        for (int32 i = 0; i < GetFrameCount(); ++i) 
		{
            m_ViewProjectionBuffers[i] = VulkanBuffer::CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vpSize);
        }
        
        m_MaterialUBODataDatas.resize(vpSize);
		for (int32 i = 0; i < MATERIAL_COUT; ++i)
		{
			uint8* dataPtr = m_MaterialUBODataDatas.data() + i * vpAlign;
			MaterialUBOData* viewProjection = (MaterialUBOData*)dataPtr;

            if (i == 0) 
            {
                viewProjection->color.Set(1.0f, 0.0f, 0.0f, 1.0f);
            } 
            else 
            {
                viewProjection->color.Set(0.0f, 1.0f, 0.0f, 1.0f);
            }
            
			viewProjection->view.SetIdentity();
			viewProjection->view.SetOrigin(Vector4(5, 5, -300.0f));
			viewProjection->view.AppendRotation(30.0f, Vector3::RightVector);
			viewProjection->view.SetInverse();

			viewProjection->projection.SetIdentity();
			viewProjection->projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetFrameWidth(), (float)GetFrameHeight(), 0.01f, 3000.0f);

			dataPtr += vpAlign;
		}

        for (int32 i = 0; i < m_ViewProjectionBuffers.size(); ++i)
        {
            m_ViewProjectionBuffers[i]->Map();
            m_ViewProjectionBuffers[i]->CopyTo(m_MaterialUBODataDatas.data(), m_MaterialUBODataDatas.size());
		    m_ViewProjectionBuffers[i]->Flush(vpSize, 0);
            m_ViewProjectionBuffers[i]->Unmap();
        }

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
        for (int32 i = 0; i < m_ModelBuffers.size(); ++i)
        {
            m_ModelBuffers[i]->Destroy();
            delete m_ModelBuffers[i];
        }
        m_ModelBuffers.clear();

        for (int32 i = 0; i < m_ViewProjectionBuffers.size(); ++i)
        {
            m_ViewProjectionBuffers[i]->Destroy();
            delete m_ViewProjectionBuffers[i];
        }
        m_ViewProjectionBuffers.clear();
    }
    
    void UpdateUniformBuffers()
    {
        float delta  = Engine::Get()->GetDeltaTime();

        uint32 bufferAlignment = GetVulkanRHI()->GetDevice()->GetLimits().minUniformBufferOffsetAlignment;

        uint32 modelAlign = Align(sizeof(ModelUBOData), bufferAlignment);
        uint32 modelSize  = modelAlign * CUBE_COUNT;

        m_ModelBuffers[m_ImageIndex]->Map();

		Vector3 offset(50.0f);

        for (uint32 x = 0; x < CUBE_SIZE; ++x)
        {
            for (uint32 y = 0; y < CUBE_SIZE; ++y)
            {
                for (uint32 z = 0; z < CUBE_SIZE; ++z)
                {
					uint32 index    = x * CUBE_SIZE * CUBE_SIZE + y * CUBE_SIZE + z;
                    uint8* dataPtr  = m_ModelUBODataDatas.data() + index * modelAlign;
					ModelUBOData* model = (ModelUBOData*)dataPtr;
					Vector3 pos     = Vector3(
						-((CUBE_SIZE * offset.x) / 2.0f) + offset.x / 2.0f + x * offset.x, 
						-((CUBE_SIZE * offset.y) / 2.0f) + offset.y / 2.0f + y * offset.y, 
						-((CUBE_SIZE * offset.z) / 2.0f) + offset.z / 2.0f + z * offset.z
					);
					model->model.SetOrigin(pos);
					model->model.AppendRotation(90.0f * delta, Vector3::UpVector);
					model->model.AppendRotation(index * delta, Vector3::RightVector);
					model->model.AppendRotation(index * delta, Vector3::ForwardVector);
                }
            }
        }
        
        m_ModelBuffers[m_ImageIndex]->CopyTo(m_ModelUBODataDatas.data(), m_ModelUBODataDatas.size());
		m_ModelBuffers[m_ImageIndex]->Flush(modelSize, 0);
        m_ModelBuffers[m_ImageIndex]->Unmap();
    }
    
    void LoadAssets()
    {
        m_Shader   = Shader::Create("assets/shaders/6_DynamicUniformBuffer/obj.vert.spv", "assets/shaders/6_DynamicUniformBuffer/obj.frag.spv");
        m_Material = std::make_shared<Material>(m_Shader);
        
		m_Renderable = MeshLoader::LoadFromFile("assets/models/suzanne.obj")[0];
    }
    
private:
    bool                            m_Ready;
    
    std::shared_ptr<Shader>         m_Shader;
    std::shared_ptr<Material>       m_Material;
    std::shared_ptr<Renderable>     m_Renderable;
    
    std::vector<uint8>              m_ModelUBODataDatas;
    std::vector<VulkanBuffer*>      m_ModelBuffers;

    std::vector<uint8>              m_MaterialUBODataDatas;
    std::vector<VulkanBuffer*>      m_ViewProjectionBuffers;
    
    std::vector<VkDescriptorSet>    m_DescriptorSets;
    VkDescriptorPool                m_DescriptorPool;
	VkDescriptorSetLayout			m_DescriptorSetLayout;
	VkPipelineLayout				m_PipelineLayout;
    VkPipeline						m_Pipeline;

    uint32                          m_ImageIndex;
};

AppModuleBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
    return new DynamicUniformBufferMode(1120, 840, "DynamicUniformBuffer", cmdLine);
}
