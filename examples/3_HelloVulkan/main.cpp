
#include <vulkan/vulkan.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string.h>
#include <vector>

#include "Common/Common.h"
#include "Sample.h"
#include "Platform/glfw/GLFWApplication.h"

class HelloVulkan : public Sample
{
public:
	HelloVulkan() : Sample()
	{

	}

	virtual ~HelloVulkan()
	{
		
	}
    
	virtual bool OnInit()
	{
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFrameBuffer();
		CreateCommandPool();
		CreateCommandBuffers();
		CreateSemaphores();
        return true;
	}
    
	virtual void OnUpdate()
	{

	}

	virtual void OnRender()
	{
		uint32_t imageIndex;
		vkAcquireNextImageKHR(m_Application->m_VKDevice, m_Application->m_VKSwapChain, std::numeric_limits<uint64_t>::max(), m_VKImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		
		VkSemaphore waitSemaphores[] = { m_VKImageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_VKCommandBuffers[imageIndex];

		VkSemaphore signalSemaphores[] = { m_VKRenderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(m_Application->m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { m_Application->m_VKSwapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		vkQueuePresentKHR(m_Application->m_GraphicsQueue, &presentInfo);
		vkQueueWaitIdle(m_Application->m_GraphicsQueue);
	}

	virtual void OnDestory()
	{
		vkDestroySemaphore(m_Application->m_VKDevice, m_VKImageAvailableSemaphore, nullptr);
		vkDestroySemaphore(m_Application->m_VKDevice, m_VKRenderFinishedSemaphore, nullptr);
		vkDestroyCommandPool(m_Application->m_VKDevice, m_VKCommandPool, nullptr);
		for (int i = 0; i < m_VKSwapchainFramebuffers.size(); ++i) {
			vkDestroyFramebuffer(m_Application->m_VKDevice, m_VKSwapchainFramebuffers[i], nullptr);
		}
		vkDestroyRenderPass(m_Application->m_VKDevice, m_VKRenderPass, nullptr);
		vkDestroyPipelineLayout(m_Application->m_VKDevice, m_VKPipelineLayout, nullptr);
		vkDestroyPipeline(m_Application->m_VKDevice, m_VKPipeline, nullptr);
	}
	
	virtual void OnDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char* layerPrefix, const char* msg)
	{
        printf("ERROR -> LayerPrefix:%s Msg:%s\n", layerPrefix, msg);
	}
    
private:
    
    std::vector<char> ReadFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }
    
    VkShaderModule createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_Application->m_VKDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }
        return shaderModule;
    }

	void CreateSemaphores()
	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		if (vkCreateSemaphore(m_Application->m_VKDevice, &semaphoreInfo, nullptr, &m_VKImageAvailableSemaphore) != VK_SUCCESS ||
			vkCreateSemaphore(m_Application->m_VKDevice, &semaphoreInfo, nullptr, &m_VKRenderFinishedSemaphore) != VK_SUCCESS) {
			printf("Failed create semaphore.\n");
		}
	}

	void CreateCommandBuffers()
	{
		m_VKCommandBuffers.resize(m_VKSwapchainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_VKCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = m_VKCommandBuffers.size();

		if (vkAllocateCommandBuffers(m_Application->m_VKDevice, &allocInfo, m_VKCommandBuffers.data()) != VK_SUCCESS) {
			printf("Failed create command buffers.\n");
		}

		for (int i = 0; i < m_VKCommandBuffers.size(); ++i) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;
			vkBeginCommandBuffer(m_VKCommandBuffers[i], &beginInfo);

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_VKRenderPass;
			renderPassInfo.framebuffer = m_VKSwapchainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = m_Application->m_VKSwapChainExtent;
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(m_VKCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(m_VKCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_VKPipeline);
			vkCmdDraw(m_VKCommandBuffers[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(m_VKCommandBuffers[i]);

			if (vkEndCommandBuffer(m_VKCommandBuffers[i]) != VK_SUCCESS) {
				printf("Failed record command.\n");
			}
		}

	}
	
	void CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = m_Application->m_GraphicsFamilyIndex;
		poolInfo.flags = 0;

		if (vkCreateCommandPool(m_Application->m_VKDevice, &poolInfo, nullptr, &m_VKCommandPool) != VK_SUCCESS) {
			printf("Failed create command pool\n");
		}
	}

	void CreateFrameBuffer()
	{
		m_VKSwapchainFramebuffers.resize(m_Application->m_VKSwapChainImageViews.size());
		for (int i = 0; i < m_VKSwapchainFramebuffers.size(); ++i) {
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = m_VKRenderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = &(m_Application->m_VKSwapChainImageViews[i]);
			framebufferInfo.width = m_Application->m_VKSwapChainExtent.width;
			framebufferInfo.height = m_Application->m_VKSwapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(m_Application->m_VKDevice, &framebufferInfo, nullptr, &(m_VKSwapchainFramebuffers[i])) != VK_SUCCESS) {
				printf("Failed create frame buffer.\n");
			}
		}
	}

	void CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = m_Application->m_VKSwapChainFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		
		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		
		if (vkCreateRenderPass(m_Application->m_VKDevice, &renderPassInfo, nullptr, &m_VKRenderPass) != VK_SUCCESS) {
			printf("Failed create render pass.\n");
		}
	}
    
    void CreateGraphicsPipeline()
    {
        auto vertShaderCode = ReadFile("../vert.spv");
        auto fragShaderCode = ReadFile("../frag.spv");
        
        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
        
        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        
        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
		
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = m_Application->m_VKSwapChainExtent.width;
		viewport.height = m_Application->m_VKSwapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = {0, 0};
		scissor.extent = m_Application->m_VKSwapChainExtent;

		VkPipelineViewportStateCreateInfo viewportStateInfo = {};
		viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateInfo.viewportCount = 1;
		viewportStateInfo.pViewports = &viewport;
		viewportStateInfo.scissorCount = 1;
		viewportStateInfo.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterInfo = {};
		rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterInfo.depthClampEnable = VK_FALSE;
		rasterInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterInfo.lineWidth = 1.0f;
		rasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterInfo.depthBiasEnable = VK_FALSE;
		rasterInfo.depthBiasConstantFactor = 0.0f;
		rasterInfo.depthBiasClamp = 0.0f;
		rasterInfo.depthBiasSlopeFactor = 0.0f;
		
		VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
		multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingInfo.sampleShadingEnable = VK_FALSE;
		multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingInfo.minSampleShading = 1.0f;
		multisamplingInfo.pSampleMask = nullptr;
		multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
		multisamplingInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};

		VkPipelineColorBlendAttachmentState colorBlendInfo = {};
		colorBlendInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendInfo.blendEnable = VK_FALSE;
		colorBlendInfo.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendInfo.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendInfo.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {};
		colorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateInfo.logicOpEnable = VK_FALSE;
		colorBlendStateInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateInfo.attachmentCount = 1;
		colorBlendStateInfo.pAttachments = &colorBlendInfo;
		colorBlendStateInfo.blendConstants[0] = 0.0f;
		colorBlendStateInfo.blendConstants[1] = 0.0f;
		colorBlendStateInfo.blendConstants[2] = 0.0f;
		colorBlendStateInfo.blendConstants[3] = 0.0f;

		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = 2;
		dynamicStateInfo.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = 0;

		if (vkCreatePipelineLayout(m_Application->m_VKDevice, &pipelineLayoutInfo, nullptr, &m_VKPipelineLayout) != VK_SUCCESS) {
			printf("Failed create pipeline layout.\n");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pViewportState = &viewportStateInfo;
		pipelineInfo.pRasterizationState = &rasterInfo;
		pipelineInfo.pMultisampleState = &multisamplingInfo;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlendStateInfo;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.layout = m_VKPipelineLayout;
		pipelineInfo.renderPass = m_VKRenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_Application->m_VKDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_VKPipeline) != VK_SUCCESS) {
			printf("Failed create pipeline.\n");
		}

		vkDestroyShaderModule(m_Application->m_VKDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(m_Application->m_VKDevice, vertShaderModule, nullptr);
    }
    
private:
	VkCommandPool m_VKCommandPool;
	VkPipeline m_VKPipeline;
	VkRenderPass m_VKRenderPass;
	VkPipelineLayout m_VKPipelineLayout;
	std::vector<VkFramebuffer> m_VKSwapchainFramebuffers;
	std::vector<VkCommandBuffer> m_VKCommandBuffers;
	VkSemaphore m_VKImageAvailableSemaphore;
	VkSemaphore m_VKRenderFinishedSemaphore;
};

int main()
{
	GLFWApplication* app = new GLFWApplication(800, 600, "HelloVulkan");
	app->AddValidationLayer("VK_LAYER_LUNARG_standard_validation");
	app->AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	app->Run(new HelloVulkan());
	delete app;
	return 0;
}
