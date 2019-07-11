#include "ImageGUIContext.h"

ImageGUIContext::ImageGUIContext(std::shared_ptr<VulkanDevice> vulkanDevice, const std::string& font)
    : m_VulkanDevice(vulkanDevice)
    , m_SampleCount(VK_SAMPLE_COUNT_1_BIT)
    , m_VertexCount(0)
    , m_IndexCount(0)
    , m_Subpass(0)
    , m_DescriptorPool(VK_NULL_HANDLE)
    , m_DescriptorSetLayout(VK_NULL_HANDLE)
    , m_DescriptorSet(VK_NULL_HANDLE)
    , m_PipelineLayout(VK_NULL_HANDLE)
    , m_Pipeline(VK_NULL_HANDLE)
    , m_FontMemory(VK_NULL_HANDLE)
    , m_FontImage(VK_NULL_HANDLE)
    , m_FontView(VK_NULL_HANDLE)
    , m_FontSampler(VK_NULL_HANDLE)
    , m_Visible(false)
    , m_Updated(false)
    , m_Scale(1.0f)
    , m_FontPath(font)
{
    
}

ImageGUIContext::~ImageGUIContext()
{
    
}

void ImageGUIContext::Init()
{
    
}

void ImageGUIContext::Destroy()
{
    
}

void ImageGUIContext::Resize(uint32 width, uint32 height)
{
    
}

bool ImageGUIContext::Update()
{
    
}

void ImageGUIContext::Draw(const VkCommandBuffer commandBuffer)
{
    
}

bool ImageGUIContext::Header(const char* caption)
{
    return false;
}

bool ImageGUIContext::CheckBox(const char* caption, bool& value)
{
    return false;
}

bool ImageGUIContext::CheckBox(const char* caption, int32& value)
{
    return false;
}

bool ImageGUIContext::InputFloat(const char* caption, float& value, float step, uint32 precision)
{
    return false;
}

bool ImageGUIContext::SliderFloat(const char* caption, float& value, float min, float max)
{
    return false;
}

bool ImageGUIContext::SliderInt(const char* caption, int32& value, int32 min, int32 max)
{
    return false;
}

bool ImageGUIContext::ComboBox(const char* caption, int32& itemindex, const std::vector<std::string>& items)
{
    return false;
}

bool ImageGUIContext::Button(const char* caption)
{
    return false;
}

void ImageGUIContext::Text(const char* formatstr, ...)
{
    
}
