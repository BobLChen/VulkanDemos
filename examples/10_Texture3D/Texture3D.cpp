
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
#include "Graphics/Texture/Texture3D.h"
#include "File/FileManager.h"
#include "Utils/VulkanUtils.h"
#include <vector>

template <typename T>
class PerlinNoise
{
public:
    PerlinNoise()
    {
        for (int32 i = 0; i < 256; ++i)
        {
            m_Permutations[i] = m_Permutations[256 + i] = MMath::RandRange(0, 256);
        }
    }
    
    T noise(T x, T y, T z)
    {
        int32 X = MMath::FloorToInt(x) & 255;
        int32 Y = MMath::FloorToInt(y) & 255;
        int32 Z = MMath::FloorToInt(z) & 255;
        
        x -= MMath::FloorToInt(x);
        y -= MMath::FloorToInt(y);
        z -= MMath::FloorToInt(z);
        
        T u = fade(x);
        T v = fade(y);
        T w = fade(z);
        
        uint32_t A  = m_Permutations[X] + Y;
        uint32_t AA = m_Permutations[A] + Z;
        uint32_t AB = m_Permutations[A + 1] + Z;
        uint32_t B  = m_Permutations[X + 1] + Y;
        uint32_t BA = m_Permutations[B] + Z;
        uint32_t BB = m_Permutations[B + 1] + Z;
        
        T res = lerp(w,
                     lerp(v,
                             lerp(u, grad(m_Permutations[AA], x, y, z), grad(m_Permutations[BA], x - 1, y, z)), lerp(u, grad(m_Permutations[AB], x, y - 1, z), grad(m_Permutations[BB], x - 1, y - 1, z))),
                     lerp(v,
                          lerp(u, grad(m_Permutations[AA + 1], x, y, z - 1), grad(m_Permutations[BA + 1], x - 1, y, z - 1)),
                          lerp(u, grad(m_Permutations[AB + 1], x, y - 1, z - 1), grad(m_Permutations[BB + 1], x - 1, y - 1, z - 1))));
        return res;
    }
    
private:
    T fade(T t)
    {
        return t * t * t * (t * (t * (T)6 - (T)15) + (T)10);
    }
    
    T lerp(T t, T a, T b)
    {
        return a + t * (b - a);
    }
    
    T grad(int hash, T x, T y, T z)
    {
        int h = hash & 15;
        T u = h < 8 ? x : y;
        T v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
private:
    uint32 m_Permutations[512];
};

template <typename T>
class FractalNoise
{
public:
    FractalNoise(const PerlinNoise<T>& perlinNoise)
    {
        m_PerlinNoise = perlinNoise;
        m_Octaves     = 6;
        m_Persistence = (T)0.5;
    }
    
    T noise(T x, T y, T z)
    {
        T sum = 0;
        T max = (T)0;
        T frequency = (T)1;
        T amplitude = (T)1;
        for (int32 i = 0; i < m_Octaves; i++)
        {
            sum += m_PerlinNoise.noise(x * frequency, y * frequency, z * frequency) * amplitude;
            max += amplitude;
            amplitude *= m_Persistence;
            frequency *= (T)2;
        }
        sum = sum / max;
        return (sum + (T)1.0) / (T)2.0;
    }
    
private:
    PerlinNoise<float> m_PerlinNoise;
    uint32 m_Octaves;
    T m_Frequency;
    T m_Amplitude;
    T m_Persistence;
};

class Texture3DMode : public AppModuleBase
{
public:
    Texture3DMode(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
    : AppModuleBase(width, height, title)
    , m_Ready(false)
    , m_ImageIndex(0)
    {
        
    }
    
    virtual ~Texture3DMode()
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
        Vector4 depth;
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
        VkPipeline pipeline = m_Material->GetPipeline(m_Renderable->GetVertexBuffer()->GetVertexInputStateInfo());
        
        for (int32 i = 0; i < m_DrawCmdBuffers.size(); ++i)
        {
            renderPassBeginInfo.framebuffer = m_FrameBuffers[i];
            VERIFYVULKANRESULT(vkBeginCommandBuffer(m_DrawCmdBuffers[i], &cmdBeginInfo));
            vkCmdBeginRenderPass(m_DrawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdSetViewport(m_DrawCmdBuffers[i], 0, 1, &viewport);
            vkCmdSetScissor(m_DrawCmdBuffers[i], 0, 1, &scissor);
            vkCmdBindDescriptorSets(m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_Shader->GetPipelineLayout(), 0, 1, &(m_DescriptorSets[i]), 0, nullptr);
            vkCmdBindPipeline(m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdBindVertexBuffers(m_DrawCmdBuffers[i], 0, 1, m_Renderable->GetVertexBuffer()->GetVKBuffers().data(), offsets);
            vkCmdBindIndexBuffer(m_DrawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetBuffer(), 0, m_Renderable->GetIndexBuffer()->GetIndexType());
            vkCmdDrawIndexed(m_DrawCmdBuffers[i], m_Renderable->GetIndexBuffer()->GetIndexCount(), 1, 0, 0, 0);
            vkCmdEndRenderPass(m_DrawCmdBuffers[i]);
            VERIFYVULKANRESULT(vkEndCommandBuffer(m_DrawCmdBuffers[i]));
        }
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
            writeDescriptorSet.dstSet            = m_DescriptorSets[i];
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            writeDescriptorSet.pBufferInfo     = &(m_MVPBuffers[i]->GetDescriptorBufferInfo());
            writeDescriptorSet.dstBinding      = 0;
            vkUpdateDescriptorSets(GetDevice(), 1, &writeDescriptorSet, 0, nullptr);
            
            writeDescriptorSet.dstSet            = m_DescriptorSets[i];
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writeDescriptorSet.pImageInfo      = &(m_Texture3D->GetDescriptorImageInfo());
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
        
        m_MVPData.depth.Set(0, 0, 0, 0);
        
        m_MVPData.model.SetIdentity();
        
        m_MVPData.view.SetIdentity();
        m_MVPData.view.SetOrigin(Vector4(0, 0, -10.0f));
        m_MVPData.view.SetInverse();
        
        m_MVPData.projection.SetIdentity();
        m_MVPData.projection.Perspective(MMath::DegreesToRadians(60.0f), (float)GetFrameWidth(), (float)GetFrameHeight(), 0.01f, 3000.0f);
        
        UpdateUniformBuffers();
    }
    
    void DestroyAssets()
    {
        m_Shader     = nullptr;
        m_Material   = nullptr;
        m_Renderable = nullptr;
		m_Texture3D  = nullptr;
        
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
        // m_MVPData.model.AppendRotation(90.0f * deltaTime, Vector3::UpVector);
        
        m_MVPData.depth.z += deltaTime;
        m_MVPData.depth.x  = (MMath::Sin(m_MVPData.depth.z) + 1) * 0.5f;
        
        m_MVPBuffers[m_ImageIndex]->Map(sizeof(m_MVPData), 0);
        m_MVPBuffers[m_ImageIndex]->CopyTo(&m_MVPData, sizeof(m_MVPData));
        m_MVPBuffers[m_ImageIndex]->Unmap();
    }
    
    uint8* GenerateNoiseData(int32 width, int32 height, int32 depth)
    {
        const uint32 texMemSize = width * height * depth;
        
        uint8 *data = new uint8[texMemSize];
        memset(data, 0, texMemSize);
        
        MLOG("Generating noise(%dx%dx%d) data...", width, height, depth);
        
        PerlinNoise<float> perlinNoise;
        FractalNoise<float> fractalNoise(perlinNoise);
        
        const float noiseScale = static_cast<float>(rand() % 10) + 4.0f;
        
#pragma omp parallel for
        for (int32 z = 0; z < depth; ++z)
        {
            for (int32 y = 0; y < height; ++y)
            {
                for (int32 x = 0; x < width; ++x)
                {
                    float nx = (float)x / (float)width;
                    float ny = (float)y / (float)height;
                    float nz = (float)z / (float)depth;
#define FRACTAL
#ifdef FRACTAL
                    float n = fractalNoise.noise(nx * noiseScale, ny * noiseScale, nz * noiseScale);
#else
                    float n = 20.0 * perlinNoise.noise(nx, ny, nz);
#endif
                    n = n - floor(n);
                    
                    data[x + y * width + z * width * height] = static_cast<uint8>(floor(n * 255));
                }
            }
        }
        
        return data;
    }
    
    void LoadNoiseTexture3D(int32 width, int32 height, int32 depth)
    {
		VkFormat format = VK_FORMAT_R8_UNORM;

        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(GetVulkanRHI()->GetDevice()->GetPhysicalHandle(), format, &formatProperties);
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT))
        {
            MLOGE("Error: Device does not support flag TRANSFER_DST for selected texture format!");
            return;
        }
        
        uint32 maxImageDimension3D(GetVulkanRHI()->GetDevice()->GetLimits().maxImageDimension3D);
        if (width > maxImageDimension3D || height > maxImageDimension3D || depth > maxImageDimension3D)
        {
            MLOGE("Error: Requested texture dimensions is greater than supported 3D texture dimension!");
            return;
        }
        
        uint8* data = GenerateNoiseData(width, height, depth);
        
		m_Texture3D = std::make_shared<Texture3D>();
		m_Texture3D->LoadFromBuffer(data, width, height, depth, format);
    }
    
    void LoadAssets()
    {
        LoadNoiseTexture3D(128, 128, 128);
        
        m_Shader   = Shader::Create("assets/shaders/10_Texture3D/texture3D.vert.spv", "assets/shaders/10_Texture3D/texture3D.frag.spv");
        m_Material = std::make_shared<Material>(m_Shader);
        
        m_Renderable = MeshLoader::LoadFromFile("assets/models/head.obj")[0];
    }
    
private:
    bool                             m_Ready;
    
    std::shared_ptr<Shader>          m_Shader;
    std::shared_ptr<Material>        m_Material;
    std::shared_ptr<Renderable>      m_Renderable;
    
	std::shared_ptr<Texture3D>       m_Texture3D;  
    UBOData                          m_MVPData;
    std::vector<VulkanBuffer*>       m_MVPBuffers;
    
    std::vector<VkDescriptorSet>     m_DescriptorSets;
    VkDescriptorPool                 m_DescriptorPool;
    
    uint32                           m_ImageIndex;
};

AppModuleBase* CreateAppMode(const std::vector<std::string>& cmdLine)
{
    return new Texture3DMode(1120, 840, "Texture3D", cmdLine);
}
