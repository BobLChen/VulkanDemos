#pragma once

#include "Engine.h"
#include "DVKCommand.h"
#include "DVKTexture.h"
#include "DVKModel.h"

#include "Common/Common.h"
#include "Math/Math.h"

#include "Vulkan/VulkanCommon.h"

#include <vector>

namespace vk_demo
{
    
    class DVKDefaultRes
    {
    private:
        DVKDefaultRes()
        {
            
        }
        
        virtual ~DVKDefaultRes()
        {
            
        }
    public:
        
        static void Init(std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer);
        
        static void Destroy();
        
    public:
        static DVKTexture*  texture2D;
        static DVKModel*    fullQuad;
        
    };
    
};
