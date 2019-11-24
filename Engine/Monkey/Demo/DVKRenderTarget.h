#pragma once

#include "Engine.h"
#include "DVKTexture.h"

#include "Common/Common.h"
#include "Math/Math.h"
#include "Math/Vector4.h"

#include "Vulkan/VulkanCommon.h"

#include <vector>
#include <unordered_map>

namespace vk_demo
{
    struct DVKRenderPassInfo
    {
        struct ColorEntry
        {
            DVKTexture*         renderTarget = nullptr;
            DVKTexture*         resolveTarget = nullptr;

            VkAttachmentLoadOp  loadAction = VK_ATTACHMENT_LOAD_OP_CLEAR;
            VkAttachmentStoreOp storeAction = VK_ATTACHMENT_STORE_OP_STORE;
        };

        struct DepthStencilEntry
        {
            DVKTexture*         depthStencilTarget = nullptr;
            DVKTexture*         resolveTarget = nullptr;
            VkAttachmentLoadOp  loadAction = VK_ATTACHMENT_LOAD_OP_CLEAR;
            VkAttachmentStoreOp storeAction = VK_ATTACHMENT_STORE_OP_STORE;
        };
        
        ColorEntry              colorRenderTargets[MaxSimultaneousRenderTargets];
        DepthStencilEntry       depthStencilRenderTarget;
        int32                   numColorRenderTargets;
		bool					multiview = false;
        
        // Color, no depth
        explicit DVKRenderPassInfo(DVKTexture* colorRT, VkAttachmentLoadOp colorLoadAction, VkAttachmentStoreOp colorStoreAction, DVKTexture* resolveRT)
        {
            numColorRenderTargets = 1;

            colorRenderTargets[0].renderTarget  = colorRT;
            colorRenderTargets[0].resolveTarget = resolveRT;
            colorRenderTargets[0].loadAction    = colorLoadAction;
            colorRenderTargets[0].storeAction   = colorStoreAction;

            depthStencilRenderTarget.depthStencilTarget = nullptr;
            depthStencilRenderTarget.resolveTarget      = nullptr;
            depthStencilRenderTarget.loadAction         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthStencilRenderTarget.storeAction        = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRenderTargets));
        }

        // Color And Depth
        explicit DVKRenderPassInfo(
            DVKTexture* colorRT, 
            VkAttachmentLoadOp colorLoadAction, 
            VkAttachmentStoreOp colorStoreAction,
            DVKTexture* depthRT,
            VkAttachmentLoadOp depthLoadAction, 
            VkAttachmentStoreOp depthStoreAction
        )
        {
            numColorRenderTargets = 1;

            colorRenderTargets[0].renderTarget  = colorRT;
            colorRenderTargets[0].resolveTarget = nullptr;
            colorRenderTargets[0].loadAction    = colorLoadAction;
            colorRenderTargets[0].storeAction   = colorStoreAction;

            depthStencilRenderTarget.depthStencilTarget = depthRT;
            depthStencilRenderTarget.resolveTarget      = nullptr;
            depthStencilRenderTarget.loadAction         = depthLoadAction;
            depthStencilRenderTarget.storeAction        = depthStoreAction;

            memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRenderTargets));
        }
        
        // MRTs, No Depth
        explicit DVKRenderPassInfo(int32 numColorRTs, DVKTexture* colorRT[], VkAttachmentLoadOp colorLoadAction, VkAttachmentStoreOp colorStoreAction)
        {
            numColorRenderTargets = numColorRTs;

            for (int32 i = 0; i < numColorRTs; ++i)
            {
                colorRenderTargets[i].renderTarget  = colorRT[i];
                colorRenderTargets[i].resolveTarget = nullptr;
                colorRenderTargets[i].loadAction    = colorLoadAction;
                colorRenderTargets[i].storeAction   = colorStoreAction;
            }

            depthStencilRenderTarget.depthStencilTarget = nullptr;
            depthStencilRenderTarget.resolveTarget      = nullptr;
            depthStencilRenderTarget.loadAction         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthStencilRenderTarget.storeAction        = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            if (numColorRTs < MaxSimultaneousRenderTargets) {
                memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRenderTargets));
            }
        }

        // MRTs And Depth
        explicit DVKRenderPassInfo(
            int32 numColorRTs, 
            DVKTexture* colorRT[], 
            VkAttachmentLoadOp colorLoadAction, 
            VkAttachmentStoreOp colorStoreAction,
            DVKTexture* depthRT,
            VkAttachmentLoadOp depthLoadAction, 
            VkAttachmentStoreOp depthStoreAction
        )
        {
            numColorRenderTargets = numColorRTs;

            for (int32 i = 0; i < numColorRTs; ++i)
            {
                colorRenderTargets[i].renderTarget  = colorRT[i];
                colorRenderTargets[i].resolveTarget = nullptr;
                colorRenderTargets[i].loadAction    = colorLoadAction;
                colorRenderTargets[i].storeAction   = colorStoreAction;
            }

            depthStencilRenderTarget.depthStencilTarget = depthRT;
            depthStencilRenderTarget.resolveTarget      = nullptr;
            depthStencilRenderTarget.loadAction         = depthLoadAction;
            depthStencilRenderTarget.storeAction        = depthStoreAction;

            if (numColorRTs < MaxSimultaneousRenderTargets) {
                memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * (MaxSimultaneousRenderTargets - numColorRenderTargets));
            }
        }

        // Depth, No Color
        explicit DVKRenderPassInfo(DVKTexture* depthRT, VkAttachmentLoadOp depthLoadAction, VkAttachmentStoreOp depthStoreAction)
        {
            numColorRenderTargets = 0;

            depthStencilRenderTarget.depthStencilTarget = depthRT;
            depthStencilRenderTarget.resolveTarget      = nullptr;
            depthStencilRenderTarget.loadAction         = depthLoadAction;
            depthStencilRenderTarget.storeAction        = depthStoreAction;

            memset(&colorRenderTargets[numColorRenderTargets], 0, sizeof(ColorEntry) * MaxSimultaneousRenderTargets);
        }
    };

    class DVKRenderTargetLayout
    {
    public:
        DVKRenderTargetLayout(const DVKRenderPassInfo& renderPassInfo);

        uint16 SetupSubpasses(VkSubpassDescription* outDescs, uint32 maxDescs, VkSubpassDependency* outDeps, uint32 maxDeps, uint32& outNumDependencies) const;
        
    public:
        VkAttachmentReference   colorReferences[MaxSimultaneousRenderTargets];
        VkAttachmentReference   depthStencilReference;
        VkAttachmentReference   resolveReferences[MaxSimultaneousRenderTargets];
        VkAttachmentReference   inputAttachments[MaxSimultaneousRenderTargets + 1];

        VkAttachmentDescription descriptions[MaxSimultaneousRenderTargets * 2 + 1];

        uint8                   numAttachmentDescriptions = 0;
        uint8                   numColorAttachments = 0;
        uint8                   numInputAttachments = 0;
        bool                    hasDepthStencil = false;
        bool                    hasResolveAttachments = false;
        VkSampleCountFlagBits   numSamples = VK_SAMPLE_COUNT_1_BIT;
        uint8                   numUsedClearValues = 0;

        VkExtent3D	            extent3D;
		bool					multiview = false;
    };

    class DVKRenderPass
    {
    public:
        DVKRenderPass(VkDevice inDevice, const DVKRenderTargetLayout& rtLayout);

        ~DVKRenderPass()
        {
            if (renderPass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(device, renderPass, VULKAN_CPU_ALLOCATOR);
                renderPass = VK_NULL_HANDLE;
            }
        }

    public:
        DVKRenderTargetLayout	layout;
        VkRenderPass		    renderPass = VK_NULL_HANDLE;
        uint32				    numUsedClearValues = 0;
        VkDevice		        device = VK_NULL_HANDLE;
    };

    class DVKFrameBuffer
    {
    public:
        DVKFrameBuffer(VkDevice device, const DVKRenderTargetLayout& rtLayout, const DVKRenderPass& renderPass, const DVKRenderPassInfo& renderPassInfo);

        ~DVKFrameBuffer()
        {
            if (frameBuffer != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(device, frameBuffer, VULKAN_CPU_ALLOCATOR);
                frameBuffer = VK_NULL_HANDLE;
            }
        }

    public:
        typedef std::vector<VkImageView> ImageViews;

        VkDevice            device = VK_NULL_HANDLE;

        VkFramebuffer       frameBuffer = VK_NULL_HANDLE;
        uint32              numColorRenderTargets = 0;
        uint32              numColorAttachments = 0;

        ImageViews          attachmentTextureViews;
        VkImage             colorRenderTargetImages[MaxSimultaneousRenderTargets];
	    VkImage             depthStencilRenderTargetImage = VK_NULL_HANDLE;

        VkExtent2D          extent2D;
    };
    
    class DVKRenderTarget
    {
    private:
        DVKRenderTarget(const DVKRenderPassInfo& inRenderPassInfo)
            : rtLayout(inRenderPassInfo)
            , renderPassInfo(inRenderPassInfo)
			, clearColor(0, 0, 0, 1)
        {
            for (int32 i = 0; i < inRenderPassInfo.numColorRenderTargets; ++i)
            {
                VkClearValue clearValue = {};
                clearValue.color        = { { clearColor.x, clearColor.y, clearColor.z, clearColor.w } };
                clearValues.push_back(clearValue);
            }
            
            if (inRenderPassInfo.depthStencilRenderTarget.depthStencilTarget)
            {
                VkClearValue clearValue = {};
                clearValue.depthStencil = { 1.0f, 0 };
                clearValues.push_back(clearValue);
            }

			colorLayout = ImageLayoutBarrier::PixelShaderRead;
			depthLayout = ImageLayoutBarrier::PixelShaderRead;
        }
        
		DVKRenderTarget(const DVKRenderPassInfo& inRenderPassInfo, Vector4 inClearColor)
			: rtLayout(inRenderPassInfo)
			, renderPassInfo(inRenderPassInfo)
			, clearColor(inClearColor)
		{
			for (int32 i = 0; i < inRenderPassInfo.numColorRenderTargets; ++i)
			{
				VkClearValue clearValue = {};
				clearValue.color        = { { clearColor.x, clearColor.y, clearColor.z, clearColor.w } };
				clearValues.push_back(clearValue);
			}

			if (inRenderPassInfo.depthStencilRenderTarget.depthStencilTarget)
			{
				VkClearValue clearValue = {};
				clearValue.depthStencil = { 1.0f, 0 };
				clearValues.push_back(clearValue);
			}

			colorLayout = ImageLayoutBarrier::PixelShaderRead;
			depthLayout = ImageLayoutBarrier::PixelShaderRead;
		}

    public:
        ~DVKRenderTarget()
        {
            if (renderPass) 
			{
                delete renderPass;
                renderPass = nullptr;
            }
            
            if (frameBuffer) 
			{
                delete frameBuffer;
                frameBuffer = nullptr;
            }
        }
        
        void BeginRenderPass(VkCommandBuffer cmdBuffer);
        
        void EndRenderPass(VkCommandBuffer cmdBuffer);

		inline VkRenderPass GetRenderPass() const
		{
			return renderPass->renderPass;
		}

		inline VkFramebuffer GetFrameBuffer() const
		{
			return frameBuffer->frameBuffer;
		}
        
        static DVKRenderTarget* Create(std::shared_ptr<VulkanDevice> vulkanDevice, const DVKRenderPassInfo& inRenderPassInfo);
        
		static DVKRenderTarget* Create(std::shared_ptr<VulkanDevice> vulkanDevice, const DVKRenderPassInfo& inRenderPassInfo, Vector4 clearColor);

    public:
        DVKRenderTargetLayout       rtLayout;
        DVKRenderPassInfo           renderPassInfo;
		Vector4						clearColor;
		
        DVKRenderPass*              renderPass = nullptr;
        DVKFrameBuffer*             frameBuffer = nullptr;
        
        VkDevice                    device = VK_NULL_HANDLE;
        VkExtent2D                  extent2D;
        std::vector<VkClearValue>   clearValues;

		ImageLayoutBarrier			colorLayout;
		ImageLayoutBarrier			depthLayout;
    };

};
