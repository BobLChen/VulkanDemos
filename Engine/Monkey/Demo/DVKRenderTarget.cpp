#include "DVKRenderTarget.h"
#include "DVKUtils.h"

namespace vk_demo
{

    // -------------- DVKRenderTargetLayout --------------
    DVKRenderTargetLayout::DVKRenderTargetLayout(const DVKRenderPassInfo& renderPassInfo)
    {
        memset(colorReferences, 0, sizeof(colorReferences));
        memset(&depthStencilReference, 0, sizeof(depthStencilReference));
        memset(resolveReferences, 0, sizeof(resolveReferences));
        memset(inputAttachments, 0, sizeof(inputAttachments));
        memset(descriptions, 0, sizeof(descriptions));
        memset(&extent3D, 0, sizeof(extent3D));

        for (int32 index = 0; index < renderPassInfo.numColorRenderTargets; ++index)
        {
            const DVKRenderPassInfo::ColorEntry& colorEntry = renderPassInfo.colorRenderTargets[index];
            DVKTexture* texture = colorEntry.renderTarget;

            extent3D.width  = texture->width;
            extent3D.height = texture->height;
            extent3D.depth  = texture->depth;
            numSamples      = texture->numSamples;

            VkAttachmentDescription& attchmentDescription = descriptions[numAttachmentDescriptions];
            attchmentDescription.samples        = numSamples;
            attchmentDescription.format         = texture->format;
            attchmentDescription.loadOp         = colorEntry.loadAction;
            attchmentDescription.storeOp        = colorEntry.storeAction;
            attchmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attchmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attchmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attchmentDescription.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorReferences[numColorAttachments].attachment = numAttachmentDescriptions;
            colorReferences[numColorAttachments].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            
            if (numSamples != VK_SAMPLE_COUNT_1_BIT)
            {
                descriptions[numAttachmentDescriptions + 1] = descriptions[numAttachmentDescriptions];
                descriptions[numAttachmentDescriptions + 1].samples = VK_SAMPLE_COUNT_1_BIT;

                resolveReferences[numColorAttachments].attachment = numAttachmentDescriptions + 1;
                resolveReferences[numColorAttachments].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

                numAttachmentDescriptions += 1;
                hasResolveAttachments      = true;
            }

            numAttachmentDescriptions += 1;
            numColorAttachments       += 1;
        }

        if (renderPassInfo.depthStencilRenderTarget.depthStencilTarget)
        {
            DVKTexture* texture = renderPassInfo.depthStencilRenderTarget.depthStencilTarget;
            VkAttachmentDescription& attchmentDescription = descriptions[numAttachmentDescriptions];

			extent3D.width  = texture->width;
			extent3D.height = texture->height;
			extent3D.depth  = texture->depth;
			numSamples      = texture->numSamples;

            attchmentDescription.samples        = texture->numSamples;
            attchmentDescription.format         = texture->format;
            attchmentDescription.loadOp         = renderPassInfo.depthStencilRenderTarget.loadAction;
            attchmentDescription.stencilLoadOp  = renderPassInfo.depthStencilRenderTarget.loadAction;
            attchmentDescription.storeOp        = renderPassInfo.depthStencilRenderTarget.storeAction;
            attchmentDescription.stencilStoreOp = renderPassInfo.depthStencilRenderTarget.storeAction;
            attchmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            attchmentDescription.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthStencilReference.attachment = numAttachmentDescriptions;
            depthStencilReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            numAttachmentDescriptions += 1;
            hasDepthStencil            = true;
        }

		multiview = renderPassInfo.multiview;
        numUsedClearValues = numAttachmentDescriptions;
    }
    
    uint16 DVKRenderTargetLayout::SetupSubpasses(VkSubpassDescription* outDescs, uint32 maxDescs, VkSubpassDependency* outDeps, uint32 maxDeps, uint32& outNumDependencies) const
    {
        memset(outDescs, 0, sizeof(outDescs[0]) * maxDescs);

        outDescs[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        outDescs[0].colorAttachmentCount    = numColorAttachments;
        outDescs[0].pColorAttachments       = numColorAttachments > 0 ? colorReferences        : nullptr;
        outDescs[0].pResolveAttachments     = hasResolveAttachments   ? resolveReferences      : nullptr;
        outDescs[0].pDepthStencilAttachment = hasDepthStencil         ? &depthStencilReference : nullptr;
        
        outNumDependencies = 0;
        return 1;
    }

    // -------------- DVKRenderPass --------------
    DVKRenderPass::DVKRenderPass(VkDevice inDevice, const DVKRenderTargetLayout& rtLayout)
        : layout(rtLayout)
        , device(inDevice)
    {
        VkSubpassDescription subpassDesc[1];
        VkSubpassDependency  subpassDep[1];

        uint32 numDependencies = 0;
        uint16 numSubpasses    = rtLayout.SetupSubpasses(subpassDesc, 1, subpassDep, 1, numDependencies);

        VkRenderPassCreateInfo renderPassCreateInfo;
        ZeroVulkanStruct(renderPassCreateInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
        renderPassCreateInfo.attachmentCount = rtLayout.numAttachmentDescriptions;
        renderPassCreateInfo.pAttachments    = rtLayout.descriptions;
        renderPassCreateInfo.subpassCount    = numSubpasses;
        renderPassCreateInfo.pSubpasses      = subpassDesc;
        renderPassCreateInfo.dependencyCount = numDependencies;
        renderPassCreateInfo.pDependencies   = subpassDep;
        
		if (rtLayout.extent3D.depth > 1 && rtLayout.multiview)
		{
			const uint32 viewMask        = (1 << rtLayout.extent3D.depth) - 1;
			const uint32 correlationMask = (1 << rtLayout.extent3D.depth) - 1;

			VkRenderPassMultiviewCreateInfo multiviewCreateInfo;
			ZeroVulkanStruct(multiviewCreateInfo, VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO);
			multiviewCreateInfo.subpassCount         = numSubpasses;
			multiviewCreateInfo.pViewMasks           = &viewMask;
			multiviewCreateInfo.correlationMaskCount = 1;
			multiviewCreateInfo.pCorrelationMasks    = &correlationMask;

			renderPassCreateInfo.pNext = &multiviewCreateInfo;
		}

        VERIFYVULKANRESULT(vkCreateRenderPass(inDevice, &renderPassCreateInfo, VULKAN_CPU_ALLOCATOR, &renderPass));
    }

    // -------------- DVKFrameBuffer --------------
    DVKFrameBuffer::DVKFrameBuffer(VkDevice inDevice, const DVKRenderTargetLayout& rtLayout, const DVKRenderPass& renderPass, const DVKRenderPassInfo& renderPassInfo)
        : device(inDevice)
    {
        numColorAttachments   = rtLayout.numColorAttachments;
        numColorRenderTargets = renderPassInfo.numColorRenderTargets;

        for (int32 index = 0; index < renderPassInfo.numColorRenderTargets; ++index)
        {
            DVKTexture* texture = renderPassInfo.colorRenderTargets[index].renderTarget;

            colorRenderTargetImages[index] = texture->image;
            // TODO:MSAAView
            attachmentTextureViews.push_back(texture->imageView);
        }

        if (rtLayout.hasDepthStencil)
        {
            DVKTexture* texture = renderPassInfo.depthStencilRenderTarget.depthStencilTarget;
            depthStencilRenderTargetImage = texture->image;
            attachmentTextureViews.push_back(texture->imageView);
        }

        VkFramebufferCreateInfo frameBufferCreateInfo;
        ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
        frameBufferCreateInfo.renderPass      = renderPass.renderPass;
        frameBufferCreateInfo.attachmentCount = attachmentTextureViews.size();
        frameBufferCreateInfo.pAttachments    = attachmentTextureViews.data();
        frameBufferCreateInfo.width           = rtLayout.extent3D.width;
        frameBufferCreateInfo.height          = rtLayout.extent3D.height;
        frameBufferCreateInfo.layers          = rtLayout.extent3D.depth;
        VERIFYVULKANRESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &frameBuffer));

        extent2D.width  = rtLayout.extent3D.width;
        extent2D.height = rtLayout.extent3D.height;
    }
    
    void DVKRenderTarget::BeginRenderPass(VkCommandBuffer commandBuffer)
    {
		for (int32 index = 0; index < renderPassInfo.numColorRenderTargets; ++index)
		{
			DVKTexture* texture = renderPassInfo.colorRenderTargets[index].renderTarget;
			VkImage image = texture->image;
			VkImageSubresourceRange subResRange = { };
			subResRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			subResRange.baseMipLevel   = 0;
			subResRange.levelCount     = 1;
			subResRange.layerCount     = texture->depth;
			subResRange.baseArrayLayer = 0;
			ImagePipelineBarrier(commandBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::ColorAttachment, subResRange);
		}

		if (renderPassInfo.depthStencilRenderTarget.depthStencilTarget)
		{
			DVKTexture* texture = renderPassInfo.depthStencilRenderTarget.depthStencilTarget;
			VkImage image = texture->image;
			VkImageSubresourceRange subResRange = { };
			subResRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			subResRange.baseMipLevel   = 0;
			subResRange.levelCount     = 1;
			subResRange.layerCount     = renderPassInfo.depthStencilRenderTarget.depthStencilTarget->depth;
			subResRange.baseArrayLayer = 0;
			ImagePipelineBarrier(commandBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::DepthStencilAttachment, subResRange);
		}

        VkViewport viewport = {};
        viewport.x        = 0;
        viewport.y        = extent2D.height;
        viewport.width    = extent2D.width;
        viewport.height   = -(float)extent2D.height;    // flip y axis
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        
        VkRect2D scissor = {};
        scissor.extent.width  = extent2D.width;
        scissor.extent.height = extent2D.height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        
        VkRenderPassBeginInfo renderPassBeginInfo;
        ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass               = renderPass->renderPass;
        renderPassBeginInfo.framebuffer              = frameBuffer->frameBuffer;
        renderPassBeginInfo.renderArea.offset.x      = 0;
        renderPassBeginInfo.renderArea.offset.y      = 0;
        renderPassBeginInfo.renderArea.extent.width  = extent2D.width;
        renderPassBeginInfo.renderArea.extent.height = extent2D.height;
        renderPassBeginInfo.clearValueCount          = clearValues.size();
        renderPassBeginInfo.pClearValues             = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer,  0, 1, &scissor);
    }
    
    void DVKRenderTarget::EndRenderPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);

		for (int32 index = 0; index < renderPassInfo.numColorRenderTargets; ++index)
		{
			DVKTexture* texture = renderPassInfo.colorRenderTargets[index].renderTarget;
			VkImage image = texture->image;
			VkImageSubresourceRange subResRange = { };
			subResRange.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
			subResRange.baseMipLevel   = 0;
			subResRange.levelCount     = 1;
			subResRange.layerCount	   = texture->depth;
			subResRange.baseArrayLayer = 0;
			ImagePipelineBarrier(commandBuffer, image, ImageLayoutBarrier::ColorAttachment, colorLayout, subResRange);
		}

		if (renderPassInfo.depthStencilRenderTarget.depthStencilTarget)
		{
			DVKTexture* texture = renderPassInfo.depthStencilRenderTarget.depthStencilTarget;
			VkImage image = texture->image;
			VkImageSubresourceRange subResRange = { };
			subResRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			subResRange.baseMipLevel   = 0;
			subResRange.levelCount     = 1;
			subResRange.layerCount     = renderPassInfo.depthStencilRenderTarget.depthStencilTarget->depth;
			subResRange.baseArrayLayer = 0;
			ImagePipelineBarrier(commandBuffer, image, ImageLayoutBarrier::DepthStencilAttachment, depthLayout, subResRange);
		}
    }
    
    DVKRenderTarget* DVKRenderTarget::Create(std::shared_ptr<VulkanDevice> vulkanDevice, const DVKRenderPassInfo& inRenderPassInfo)
    {
        VkDevice device = vulkanDevice->GetInstanceHandle();
        
        DVKRenderTarget* renderTarget = new DVKRenderTarget(inRenderPassInfo);
        renderTarget->device          = device;
        renderTarget->renderPass      = new DVKRenderPass(device, renderTarget->rtLayout);
        renderTarget->frameBuffer     = new DVKFrameBuffer(device, renderTarget->rtLayout, *(renderTarget->renderPass), inRenderPassInfo);
        renderTarget->extent2D        = renderTarget->frameBuffer->extent2D;
        
        return renderTarget;
    }

	DVKRenderTarget* DVKRenderTarget::Create(std::shared_ptr<VulkanDevice> vulkanDevice, const DVKRenderPassInfo& inRenderPassInfo, Vector4 clearColor)
	{
		VkDevice device = vulkanDevice->GetInstanceHandle();

		DVKRenderTarget* renderTarget = new DVKRenderTarget(inRenderPassInfo, clearColor);
		renderTarget->device          = device;
		renderTarget->renderPass      = new DVKRenderPass(device, renderTarget->rtLayout);
		renderTarget->frameBuffer     = new DVKFrameBuffer(device, renderTarget->rtLayout, *(renderTarget->renderPass), inRenderPassInfo);
		renderTarget->extent2D        = renderTarget->frameBuffer->extent2D;

		return renderTarget;
	}
    
};
