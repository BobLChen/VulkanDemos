#include "Common/Common.h"
#include "Common/Log.h"

#include "Application/AppModuleBase.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Demo/FileManager.h"

#include "Vulkan/VulkanCommon.h"

#include <vector>

class TriangleModule : public AppModuleBase
{
public:
	TriangleModule(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: AppModuleBase(width, height, title)
		, m_Ready(false)
		, m_IndicesCount(0)
	{
        
	}
    
	virtual ~TriangleModule()
	{

	}

	virtual bool PreInit() override
	{
		return true;
	}

	virtual bool Init() override
	{
        CreateDepthStencil();
        CreateRenderPass();
        CreateFrameBuffers();
		CreateSemaphores();
		CreateFences();
		CreateCommandBuffers();
		CreateMeshBuffers();
		CreateUniformBuffers();
        CreateDescriptorPool();
		CreateDescriptorSetLayout();
        CreateDescriptorSet();
		CreatePipelines();
		SetupCommandBuffers();

		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
        DestroyFrameBuffers();
        DestoryRenderPass();
        DestoryDepthStencil();
		DestroyCommandBuffers();
        DestroyDescriptorSetLayout();
		DestroyDescriptorPool();
		DestroyPipelines();
		DestroyUniformBuffers();
		DestroyMeshBuffers();
		DestorySemaphores();
		DestroyFences();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

private:

	struct GPUBuffer
	{
		VkDeviceMemory 	memory;
		VkBuffer 		buffer;

		GPUBuffer()
			: memory(VK_NULL_HANDLE)
			, buffer(VK_NULL_HANDLE)
		{

		}
	};
	
	typedef GPUBuffer 	IndexBuffer;
	typedef GPUBuffer 	VertexBuffer;
	typedef GPUBuffer 	UBOBuffer;

	struct Vertex
	{
		float position[3];
		float color[3];
	};

	struct UBOData
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};

	VkShaderModule LoadSPIPVShader(const std::string& filepath)
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
		uint8* dataPtr  = nullptr;
		uint32 dataSize = 0;
		FileManager::ReadFile(filepath, dataPtr, dataSize);

		VkShaderModuleCreateInfo moduleCreateInfo;
		ZeroVulkanStruct(moduleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
		moduleCreateInfo.codeSize = dataSize;
		moduleCreateInfo.pCode    = (uint32_t*)dataPtr;

		VkShaderModule shaderModule;
		VERIFYVULKANRESULT(vkCreateShaderModule(device, &moduleCreateInfo, VULKAN_CPU_ALLOCATOR, &shaderModule));
		delete[] dataPtr;
		
		return shaderModule;
	}
    
	void Draw(float time, float delta)
	{
		UpdateUniformBuffers(time, delta);
        
        VkQueue queue = GetVulkanRHI()->GetDevice()->GetPresentQueue()->GetHandle();
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        int32 backBufferIndex = GetVulkanRHI()->GetSwapChain()->AcquireImageIndex(&m_PresentComplete);

        VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        
		VkSubmitInfo submitInfo = {};
		submitInfo.sType 				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask 	= &waitStageMask;									
		submitInfo.pWaitSemaphores 		= &m_PresentComplete;
		submitInfo.waitSemaphoreCount 	= 1;
		submitInfo.pSignalSemaphores 	= &m_RenderComplete;
		submitInfo.signalSemaphoreCount = 1;											
		submitInfo.pCommandBuffers 		= &(m_CommandBuffers[backBufferIndex]);
		submitInfo.commandBufferCount 	= 1;												

		// 提交绘制命令
        vkResetFences(device, 1, &(m_Fences[backBufferIndex]));
		VERIFYVULKANRESULT(vkQueueSubmit(queue, 1, &submitInfo, m_Fences[backBufferIndex]));
        vkWaitForFences(device, 1, &(m_Fences[backBufferIndex]), true, 200 * 1000 * 1000);
        
        // present
        GetVulkanRHI()->GetSwapChain()->Present(
            GetVulkanRHI()->GetDevice()->GetGraphicsQueue(),
            GetVulkanRHI()->GetDevice()->GetPresentQueue(),
            &m_RenderComplete
        );
	}

	void SetupCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

		VkClearValue clearValues[2];
		clearValues[0].color        = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

        int32 fwidth  = GetVulkanRHI()->GetSwapChain()->GetWidth();
        int32 fheight = GetVulkanRHI()->GetSwapChain()->GetHeight();
        
		VkRenderPassBeginInfo renderPassBeginInfo;
		ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = m_RenderPass;
		renderPassBeginInfo.clearValueCount = 2;
		renderPassBeginInfo.pClearValues    = clearValues;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width  = fwidth;
        renderPassBeginInfo.renderArea.extent.height = fheight;
        
		for (int32 i = 0; i < m_CommandBuffers.size(); ++i)
		{
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];
            
			VkViewport viewport = {};
			viewport.x        = 0;
			viewport.y        = fheight;
            viewport.width    = (float)fwidth;
            viewport.height   = -(float)fheight;    // flip y axis
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
            
			VkRect2D scissor = {};
            scissor.extent.width  = fwidth;
            scissor.extent.height = fheight;
			scissor.offset.x      = 0;
			scissor.offset.y      = 0;
            
			VkDeviceSize offsets[1] = { 0 };
            
			VERIFYVULKANRESULT(vkBeginCommandBuffer(m_CommandBuffers[i], &cmdBeginInfo));
            
			vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(m_CommandBuffers[i], 0, 1, &viewport);
			vkCmdSetScissor(m_CommandBuffers[i], 0, 1, &scissor);
			vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, nullptr);
			vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
			vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, &m_VertexBuffer.buffer, offsets);
			vkCmdBindIndexBuffer(m_CommandBuffers[i], m_IndicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(m_CommandBuffers[i], m_IndicesCount, 1, 0, 0, 0);
			vkCmdEndRenderPass(m_CommandBuffers[i]);
            
			VERIFYVULKANRESULT(vkEndCommandBuffer(m_CommandBuffers[i]));
		}
	}
    
    void CreateFrameBuffers() override
    {
        DestroyFrameBuffers();
        
        int32 fwidth    = GetVulkanRHI()->GetSwapChain()->GetWidth();
        int32 fheight   = GetVulkanRHI()->GetSwapChain()->GetHeight();
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
        VkImageView attachments[2];
        attachments[1] = m_DepthStencilView;
        
        VkFramebufferCreateInfo frameBufferCreateInfo;
        ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
        frameBufferCreateInfo.renderPass      = m_RenderPass;
        frameBufferCreateInfo.attachmentCount = 2;
        frameBufferCreateInfo.pAttachments    = attachments;
        frameBufferCreateInfo.width           = fwidth;
        frameBufferCreateInfo.height          = fheight;
        frameBufferCreateInfo.layers          = 1;
        
        const std::vector<VkImageView>& backbufferViews = GetVulkanRHI()->GetBackbufferViews();
        
        m_FrameBuffers.resize(backbufferViews.size());
        for (uint32 i = 0; i < m_FrameBuffers.size(); ++i) {
            attachments[0] = backbufferViews[i];
            VERIFYVULKANRESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
        }
    }
    
    void CreateDepthStencil() override
    {
        DestoryDepthStencil();
        
        int32 fwidth    = GetVulkanRHI()->GetSwapChain()->GetWidth();
        int32 fheight   = GetVulkanRHI()->GetSwapChain()->GetHeight();
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
        VkImageCreateInfo imageCreateInfo;
        ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
        imageCreateInfo.imageType   = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format      = PixelFormatToVkFormat(m_DepthFormat, false);
        imageCreateInfo.extent      = { (uint32)fwidth, (uint32)fheight, 1 };
        imageCreateInfo.mipLevels   = 1;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.samples     = m_SampleCount;
        imageCreateInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageCreateInfo.flags       = 0;
        VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilImage));
        
        VkImageViewCreateInfo imageViewCreateInfo;
        ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format   = PixelFormatToVkFormat(m_DepthFormat, false);
        imageViewCreateInfo.flags    = 0;
        imageViewCreateInfo.image    = m_DepthStencilImage;
        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;
        
        VkMemoryRequirements memRequire;
        vkGetImageMemoryRequirements(device, imageViewCreateInfo.image, &memRequire);
        uint32 memoryTypeIndex = 0;
        VERIFYVULKANRESULT(GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memRequire.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex));
        
        VkMemoryAllocateInfo memAllocateInfo;
        ZeroVulkanStruct(memAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
        memAllocateInfo.allocationSize  = memRequire.size;
        memAllocateInfo.memoryTypeIndex = memoryTypeIndex;
        vkAllocateMemory(device, &memAllocateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilMemory);
        vkBindImageMemory(device, m_DepthStencilImage, m_DepthStencilMemory, 0);
        
        VERIFYVULKANRESULT(vkCreateImageView(device, &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DepthStencilView));
    }
    
    void CreateRenderPass() override
    {
        DestoryRenderPass();
        
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        PixelFormat pixelFormat = GetVulkanRHI()->GetPixelFormat();
        
        std::vector<VkAttachmentDescription> attachments(2);
        // color attachment
        attachments[0].format         = PixelFormatToVkFormat(pixelFormat, false);
        attachments[0].samples        = m_SampleCount;
        attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        // depth stencil attachment
        attachments[1].format         = PixelFormatToVkFormat(m_DepthFormat, false);
        attachments[1].samples        = m_SampleCount;
        attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        VkAttachmentReference colorReference = { };
        colorReference.attachment = 0;
        colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        
        VkAttachmentReference depthReference = { };
        depthReference.attachment = 1;
        depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        
        VkSubpassDescription subpassDescription = { };
        subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount    = 1;
        subpassDescription.pColorAttachments       = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;
        subpassDescription.pResolveAttachments     = nullptr;
        subpassDescription.inputAttachmentCount    = 0;
        subpassDescription.pInputAttachments       = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments    = nullptr;
        
        std::vector<VkSubpassDependency> dependencies(2);
        dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass      = 0;
        dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        
        dependencies[1].srcSubpass      = 0;
        dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        
        VkRenderPassCreateInfo renderPassInfo;
        ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpassDescription;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies   = dependencies.data();
        VERIFYVULKANRESULT(vkCreateRenderPass(device, &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));
    }
    
    void DestroyFrameBuffers() override
    {
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        for (int32 i = 0; i < m_FrameBuffers.size(); ++i) {
            vkDestroyFramebuffer(device, m_FrameBuffers[i], VULKAN_CPU_ALLOCATOR);
        }
        m_FrameBuffers.clear();
    }
    
    void DestoryRenderPass() override
    {
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        if (m_RenderPass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device, m_RenderPass, VULKAN_CPU_ALLOCATOR);
            m_RenderPass = VK_NULL_HANDLE;
        }
    }
    
    void DestoryDepthStencil() override
    {
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
        if (m_DepthStencilMemory != VK_NULL_HANDLE) {
            vkFreeMemory(device, m_DepthStencilMemory, VULKAN_CPU_ALLOCATOR);
            m_DepthStencilMemory = VK_NULL_HANDLE;
        }
        
        if (m_DepthStencilView != VK_NULL_HANDLE) {
            vkDestroyImageView(device, m_DepthStencilView, VULKAN_CPU_ALLOCATOR);
            m_DepthStencilView = VK_NULL_HANDLE;
        }
        
        if (m_DepthStencilImage != VK_NULL_HANDLE) {
            vkDestroyImage(device, m_DepthStencilImage, VULKAN_CPU_ALLOCATOR);
            m_DepthStencilImage = VK_NULL_HANDLE;
        }
    }
    
	void CreateDescriptorSet()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
		VkDescriptorSetAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
		allocInfo.descriptorPool     = m_DescriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts        = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet));
        
		VkWriteDescriptorSet writeDescriptorSet;
		ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		writeDescriptorSet.dstSet          = m_DescriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo     = &m_MVPDescriptor;
		writeDescriptorSet.dstBinding      = 0;
		vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
	}
    
	void CreateDescriptorPool()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
		VkDescriptorPoolSize poolSize = {};
		poolSize.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = 1;
        
		VkDescriptorPoolCreateInfo descriptorPoolInfo;
		ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolInfo.poolSizeCount = 1;
		descriptorPoolInfo.pPoolSizes    = &poolSize;
		descriptorPoolInfo.maxSets       = 1;
		VERIFYVULKANRESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));
	}
    
	void DestroyDescriptorPool()
	{
		vkDestroyDescriptorPool(GetVulkanRHI()->GetDevice()->GetInstanceHandle(), m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}
    
	void CreatePipelines()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
        VkPipelineCacheCreateInfo createInfo;
        ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);
        VERIFYVULKANRESULT(vkCreatePipelineCache(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineCache));
        
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
		multisampleState.rasterizationSamples = m_SampleCount;
		multisampleState.pSampleMask 		  = nullptr;
		
		// (triangle.vert):
		// layout (location = 0) in vec3 inPos;
		// layout (location = 1) in vec3 inColor;
		// Attribute location 0: Position
		// Attribute location 1: Color
		// vertex input bindding
		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding   = 0; // Vertex Buffer 0
		vertexInputBinding.stride    = sizeof(Vertex); // Position + Color
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs(2);
		// position
		vertexInputAttributs[0].binding  = 0;
        vertexInputAttributs[0].location = 0; // triangle.vert : layout (location = 0)
		vertexInputAttributs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[0].offset   = 0;
		// color
		vertexInputAttributs[1].binding  = 0;
		vertexInputAttributs[1].location = 1; // triangle.vert : layout (location = 1)
		vertexInputAttributs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributs[1].offset   = 12;
		
		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount   = 1;
		vertexInputState.pVertexBindingDescriptions      = &vertexInputBinding;
		vertexInputState.vertexAttributeDescriptionCount = 2;
		vertexInputState.pVertexAttributeDescriptions    = vertexInputAttributs.data();
        
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
		ZeroVulkanStruct(shaderStages[0], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		ZeroVulkanStruct(shaderStages[1], VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
		shaderStages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = LoadSPIPVShader("assets/shaders/2_Triangle/triangle.vert.spv");
		shaderStages[0].pName  = "main";
		shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = LoadSPIPVShader("assets/shaders/2_Triangle/triangle.frag.spv");
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
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(device, m_PipelineCache, 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));
		
		vkDestroyShaderModule(device, shaderStages[0].module, VULKAN_CPU_ALLOCATOR);
		vkDestroyShaderModule(device, shaderStages[1].module, VULKAN_CPU_ALLOCATOR);
	}
    
	void DestroyPipelines()
	{
		VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		vkDestroyPipeline(device, m_Pipeline, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineCache(device, m_PipelineCache, VULKAN_CPU_ALLOCATOR);
	}
	
	void CreateDescriptorSetLayout()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
		VkDescriptorSetLayoutBinding layoutBinding;
		layoutBinding.binding 			 = 0;
		layoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.descriptorCount    = 1;
		layoutBinding.stageFlags 		 = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.pImmutableSamplers = nullptr;
        
		VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
		ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
		descSetLayoutInfo.bindingCount = 1;
		descSetLayoutInfo.pBindings    = &layoutBinding;
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));
        
		VkPipelineLayoutCreateInfo pipeLayoutInfo;
		ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipeLayoutInfo.setLayoutCount = 1;
		pipeLayoutInfo.pSetLayouts    = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));
	}
    
    void DestroyCommandBuffers()
    {
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        for (int32 i = 0; i < m_CommandBuffers.size(); ++i) {
            vkFreeCommandBuffers(device, m_CommandPool, 1, &(m_CommandBuffers[i]));
        }

		vkDestroyCommandPool(device, m_CommandPool, VULKAN_CPU_ALLOCATOR);
    }
    
	void CreateCommandBuffers()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
		VkCommandPoolCreateInfo cmdPoolInfo;
		ZeroVulkanStruct(cmdPoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
		cmdPoolInfo.queueFamilyIndex = GetVulkanRHI()->GetDevice()->GetPresentQueue()->GetFamilyIndex();
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VERIFYVULKANRESULT(vkCreateCommandPool(device, &cmdPoolInfo, VULKAN_CPU_ALLOCATOR, &m_CommandPool));
        
        VkCommandBufferAllocateInfo cmdBufferInfo;
        ZeroVulkanStruct(cmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
        cmdBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdBufferInfo.commandBufferCount = 1;
        cmdBufferInfo.commandPool        = m_CommandPool;
        
        m_CommandBuffers.resize(GetVulkanRHI()->GetSwapChain()->GetBackBufferCount());
        for (int32 i = 0; i < m_CommandBuffers.size(); ++i) {
            vkAllocateCommandBuffers(device, &cmdBufferInfo, &(m_CommandBuffers[i]));
        }
	}
    
	void DestroyDescriptorSetLayout()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
	}
	
	void UpdateUniformBuffers(float time, float delta)
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		m_MVPData.model.AppendRotation(90.0f * delta, Vector3::UpVector);
		uint8_t *pData = nullptr;
		VERIFYVULKANRESULT(vkMapMemory(device, m_MVPBuffer.memory, 0, sizeof(UBOData), 0, (void**)&pData));
		std::memcpy(pData, &m_MVPData, sizeof(UBOData));
		vkUnmapMemory(device, m_MVPBuffer.memory);
	}

	void CreateUniformBuffers()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
		VkBufferCreateInfo bufferInfo;
		ZeroVulkanStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		bufferInfo.size  = sizeof(UBOData);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &bufferInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.buffer));
        
		VkMemoryRequirements memReqInfo;
		vkGetBufferMemoryRequirements(device, m_MVPBuffer.buffer, &memReqInfo);
		uint32 memoryTypeIndex = 0;
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);

		VkMemoryAllocateInfo allocInfo;
		ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		allocInfo.allocationSize  = memReqInfo.size;
		allocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &allocInfo, VULKAN_CPU_ALLOCATOR, &m_MVPBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, m_MVPBuffer.buffer, m_MVPBuffer.memory, 0));
        
		m_MVPDescriptor.buffer = m_MVPBuffer.buffer;
		m_MVPDescriptor.offset = 0;
		m_MVPDescriptor.range  = sizeof(UBOData);
        
		m_MVPData.model.SetIdentity();
		m_MVPData.model.SetOrigin(Vector3(0, 0, 0));
        
		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector4(0, 0, -2.5f));
		m_MVPData.view.SetInverse();
        
		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(MMath::DegreesToRadians(75.0f), (float)GetWidth(), (float)GetHeight(), 0.01f, 3000.0f);
	}
	
	void DestroyUniformBuffers()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        
		vkDestroyBuffer(device, m_MVPBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, m_MVPBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void CreateMeshBuffers()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        VkQueue queue   = GetVulkanRHI()->GetDevice()->GetPresentQueue()->GetHandle();
        
		// 顶点数据
		std::vector<Vertex> vertices = {
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};
        
		// 索引数据
		std::vector<uint16> indices = { 0, 1, 2 };
		m_IndicesCount = (uint32)indices.size();
        
		// 顶点数据以及索引数据在整个生命周期中几乎不会发生改变，因此最佳的方式是将这些数据存储到GPU的内存中。
		// 存储到GPU内存也能加快GPU的访问。为了存储到GPU内存中，需要如下几个步骤。
		// 1、在主机端(Host)创建一个Buffer
		// 2、将数据拷贝至该Buffer
		// 3、在GPU端(Local Device)创建一个Buffer
		// 4、通过Transfer簇将数据从主机端拷贝至GPU端
		// 5、删除主基端(Host)的Buffer
		// 6、使用GPU端(Local Device)的Buffer进行渲染
		VertexBuffer tempVertexBuffer;
		IndexBuffer  tempIndexBuffer;
        
		void* dataPtr = nullptr;
		VkMemoryRequirements memReqInfo;
		VkMemoryAllocateInfo memAllocInfo;
		ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
        
		// vertex buffer
		VkBufferCreateInfo vertexBufferInfo;
		ZeroVulkanStruct(vertexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		vertexBufferInfo.size  = vertices.size() * sizeof(Vertex);
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.buffer));
        
		vkGetBufferMemoryRequirements(device, tempVertexBuffer.buffer, &memReqInfo);
		uint32 memoryTypeIndex = 0;
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, tempVertexBuffer.buffer, tempVertexBuffer.memory, 0));
        
		VERIFYVULKANRESULT(vkMapMemory(device, tempVertexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, vertices.data(), vertexBufferInfo.size);
		vkUnmapMemory(device, tempVertexBuffer.memory);
        
		// local device vertex buffer
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.buffer));
        
		vkGetBufferMemoryRequirements(device, m_VertexBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, m_VertexBuffer.buffer, m_VertexBuffer.memory, 0));
        
		// index buffer
		VkBufferCreateInfo indexBufferInfo;
		ZeroVulkanStruct(indexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		indexBufferInfo.size  = m_IndicesCount * sizeof(uint16);
		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.buffer));
        
		vkGetBufferMemoryRequirements(device, tempIndexBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, tempIndexBuffer.buffer, tempIndexBuffer.memory, 0));
        
		VERIFYVULKANRESULT(vkMapMemory(device, tempIndexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, indices.data(), indexBufferInfo.size);
		vkUnmapMemory(device, tempIndexBuffer.memory);
		
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.buffer));
		
		vkGetBufferMemoryRequirements(device, m_IndicesBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(device, m_IndicesBuffer.buffer, m_IndicesBuffer.memory, 0));
        
		VkCommandBuffer xferCmdBuffer;
		// gfx queue自带transfer功能，为了优化需要使用专有的xfer queue。这里为了简单，先将就用。
		VkCommandBufferAllocateInfo xferCmdBufferInfo;
		ZeroVulkanStruct(xferCmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
        xferCmdBufferInfo.commandPool        = m_CommandPool;
		xferCmdBufferInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		xferCmdBufferInfo.commandBufferCount = 1;
		VERIFYVULKANRESULT(vkAllocateCommandBuffers(device, &xferCmdBufferInfo, &xferCmdBuffer));
        
		// 开始录制命令
		VkCommandBufferBeginInfo cmdBufferBeginInfo;
		ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(xferCmdBuffer, &cmdBufferBeginInfo));
		
		VkBufferCopy copyRegion = {};
		copyRegion.size = vertices.size() * sizeof(Vertex);
		vkCmdCopyBuffer(xferCmdBuffer, tempVertexBuffer.buffer, m_VertexBuffer.buffer, 1, &copyRegion);
		
		copyRegion.size = indices.size() * sizeof(uint16);
		vkCmdCopyBuffer(xferCmdBuffer, tempIndexBuffer.buffer, m_IndicesBuffer.buffer, 1, &copyRegion);
        
		// 结束录制
		VERIFYVULKANRESULT(vkEndCommandBuffer(xferCmdBuffer));
		
		// 提交命令，并且等待命令执行完毕。
		VkSubmitInfo submitInfo;
		ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers    = &xferCmdBuffer;

		VkFenceCreateInfo fenceInfo;
		ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceInfo.flags = 0;
        
		VkFence fence = VK_NULL_HANDLE;
		VERIFYVULKANRESULT(vkCreateFence(device, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));
		VERIFYVULKANRESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
		VERIFYVULKANRESULT(vkWaitForFences(device, 1, &fence, VK_TRUE, MAX_int64));
        
		vkDestroyFence(device, fence, VULKAN_CPU_ALLOCATOR);
		vkFreeCommandBuffers(device, m_CommandPool, 1, &xferCmdBuffer);
        
		vkDestroyBuffer(device, tempVertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, tempVertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(device, tempIndexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, tempIndexBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}
    
	void DestroyMeshBuffers()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		vkDestroyBuffer(device, m_VertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, m_VertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(device, m_IndicesBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, m_IndicesBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}
    
	void CreateSemaphores()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		VkSemaphoreCreateInfo createInfo;
		ZeroVulkanStruct(createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		vkCreateSemaphore(device, &createInfo, VULKAN_CPU_ALLOCATOR, &m_RenderComplete);
	}
    
	void DestorySemaphores()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		vkDestroySemaphore(device, m_RenderComplete, VULKAN_CPU_ALLOCATOR);
	}

	void CreateFences()
	{
        VkDevice device  = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
        int32 frameCount = GetVulkanRHI()->GetSwapChain()->GetBackBufferCount();
        
		VkFenceCreateInfo fenceCreateInfo;
		ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        
        m_Fences.resize(frameCount);
		for (int32 i = 0; i < m_Fences.size(); ++i) {
			VERIFYVULKANRESULT(vkCreateFence(device, &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
		}
	}
    
	void DestroyFences()
	{
        VkDevice device = GetVulkanRHI()->GetDevice()->GetInstanceHandle();
		for (int32 i = 0; i < m_Fences.size(); ++i) {
			vkDestroyFence(device, m_Fences[i], VULKAN_CPU_ALLOCATOR);
		}
	}

private:
	bool 							m_Ready = false;
    
    std::vector<VkFramebuffer>      m_FrameBuffers;
    
    VkImage                         m_DepthStencilImage = VK_NULL_HANDLE;
    VkImageView                     m_DepthStencilView = VK_NULL_HANDLE;
    VkDeviceMemory                  m_DepthStencilMemory = VK_NULL_HANDLE;
    
    VkRenderPass                    m_RenderPass = VK_NULL_HANDLE;
    VkSampleCountFlagBits           m_SampleCount = VK_SAMPLE_COUNT_1_BIT;
    PixelFormat                     m_DepthFormat = PF_D24;
    
	VkCommandPool					m_CommandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer>	m_CommandBuffers;

	std::vector<VkFence> 			m_Fences;
	VkSemaphore 					m_PresentComplete = VK_NULL_HANDLE;
	VkSemaphore 					m_RenderComplete = VK_NULL_HANDLE;

	VertexBuffer 					m_VertexBuffer;
	IndexBuffer 					m_IndicesBuffer;
	UBOBuffer 						m_MVPBuffer;
	UBOData 						m_MVPData;

	VkDescriptorBufferInfo 			m_MVPDescriptor;
	
    VkDescriptorSetLayout 			m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet 				m_DescriptorSet = VK_NULL_HANDLE;
	VkPipelineLayout 				m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorPool                m_DescriptorPool = VK_NULL_HANDLE;
    
    VkPipeline 						m_Pipeline = VK_NULL_HANDLE;
    VkPipelineCache                 m_PipelineCache = VK_NULL_HANDLE;
	
	uint32 							m_IndicesCount = 0;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<TriangleModule>(1400, 900, "Triangle", cmdLine);
}
