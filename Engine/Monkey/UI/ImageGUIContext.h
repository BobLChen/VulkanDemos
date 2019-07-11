#pragma once

#include "Common/Common.h"
#include "Math/Vector2.h"
#include "Vulkan/VulkanCommon.h"

#include "imgui.h"

class ImageGUIContext
{
public:
    ImageGUIContext(std::shared_ptr<VulkanDevice> vulkanDevice, const std::string& font);
    
    virtual ~ImageGUIContext();
    
protected:
    struct UIBuffer
    {
        VkBuffer        buffer;
        VkDeviceMemory  memory;
        
        UIBuffer()
            : buffer(VK_NULL_HANDLE)
            , memory(VK_NULL_HANDLE)
        {
            
        }
    };
    
    struct PushConstBlock
    {
        Vector2 scale;
        Vector2 translate;
    };
    
    void Init();
    
    void Destroy();
    
    void Resize(uint32 width, uint32 height);
    
    bool Update();
    
    void Draw(const VkCommandBuffer commandBuffer);
    
    bool Header(const char* caption);
    
    bool CheckBox(const char* caption, bool& value);
    
    bool CheckBox(const char* caption, int32& value);
    
    bool InputFloat(const char* caption, float& value, float step, uint32 precision);
    
    bool SliderFloat(const char* caption, float& value, float min, float max);
    
    bool SliderInt(const char* caption, int32& value, int32 min, int32 max);
    
    bool ComboBox(const char* caption, int32& itemindex, const std::vector<std::string>& items);
    
    bool Button(const char* caption);
    
    void Text(const char* formatstr, ...);
    
protected:
    
    std::shared_ptr<VulkanDevice>       m_VulkanDevice;
    VkSampleCountFlagBits               m_SampleCount;
    
    UIBuffer                m_VertexBuffer;
    UIBuffer                m_IndexBuffer;
    int32                   m_VertexCount;
    int32                   m_IndexCount;
    
    int32                   m_Subpass;
    
    VkDescriptorPool        m_DescriptorPool;
    VkDescriptorSetLayout   m_DescriptorSetLayout;
    VkDescriptorSet         m_DescriptorSet;
    VkPipelineLayout        m_PipelineLayout;
    VkPipeline              m_Pipeline;
    
    VkDeviceMemory          m_FontMemory;
    VkImage                 m_FontImage;
    VkImageView             m_FontView;
    VkSampler               m_FontSampler;
    
    PushConstBlock          m_PushData;
    
    bool                    m_Visible;
    bool                    m_Updated;
    float                   m_Scale;
    
    std::string             m_FontPath;
};
