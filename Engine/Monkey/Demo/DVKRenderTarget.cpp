#include "DVKRenderTarget.h"

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
            attchmentDescription.initialLayout  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            attchmentDescription.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorReferences[numColorAttachments].attachment = numAttachmentDescriptions;
            colorReferences[numColorAttachments].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            
            if (numSamples != VK_SAMPLE_COUNT_1_BIT)
            {
                descriptions[numAttachmentDescriptions + 1] = descriptions[numAttachmentDescriptions];
                descriptions[numAttachmentDescriptions + 1].samples = VK_SAMPLE_COUNT_1_BIT;

                resolveReferences[numColorAttachments].attachment = numAttachmentDescriptions + 1;
                resolveReferences[numColorAttachments].layout     = VK_IMAGE_LAYOUT_GENERAL;

                numAttachmentDescriptions += 1;
                hasResolveAttachments      = true;
            }

            numAttachmentDescriptions += 1;
            numColorAttachments       += 1;
        }

        if (renderPassInfo.depthStencilRenderTarget.depthStencilTarget)
        {
            VkImageLayout depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // TODO:VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
            DVKTexture* texture = renderPassInfo.depthStencilRenderTarget.depthStencilTarget;
            VkAttachmentDescription& attchmentDescription = descriptions[numAttachmentDescriptions];

            attchmentDescription.samples        = texture->numSamples;
            attchmentDescription.format         = texture->format;
            attchmentDescription.loadOp         = renderPassInfo.depthStencilRenderTarget.loadAction;
            attchmentDescription.stencilLoadOp  = renderPassInfo.depthStencilRenderTarget.loadAction;
            attchmentDescription.storeOp        = renderPassInfo.depthStencilRenderTarget.storeAction;
            attchmentDescription.stencilStoreOp = renderPassInfo.depthStencilRenderTarget.storeAction;
            attchmentDescription.initialLayout  = depthStencilLayout;
            attchmentDescription.finalLayout    = depthStencilLayout;

            depthStencilReference.attachment = numAttachmentDescriptions;
            depthStencilReference.layout     = depthStencilLayout;

            numAttachmentDescriptions += 1;
            hasDepthStencil            = true;
        }

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

        VERIFYVULKANRESULT(vkCreateRenderPass(inDevice, &renderPassCreateInfo, VULKAN_CPU_ALLOCATOR, &renderPass));
    }

    // -------------- DVKFrameBuffer --------------
    DVKFrameBuffer::DVKFrameBuffer(VkDevice inDevice, const DVKRenderTargetLayout& rtLayout, const DVKRenderPass& renderPass, const DVKRenderPassInfo& renderPassInfo)
        : device(inDevice)
    {
        attachmentTextureViews.resize(rtLayout.numAttachmentDescriptions);
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

};