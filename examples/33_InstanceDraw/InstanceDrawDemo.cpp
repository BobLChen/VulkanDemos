#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

#define INSTANCE_COUNT 20480

class InstanceDrawDemo : public DemoBase
{
public:
	InstanceDrawDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~InstanceDrawDemo()
	{

	}

	virtual bool PreInit() override
	{
		return true;
	}

	virtual bool Init() override
	{
		DemoBase::Setup();
		DemoBase::Prepare();

        LoadAssets();
		InitParmas();
		CreateGUI();
		
		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
		DemoBase::Release();

		DestroyAssets();
		DestroyGUI();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

private:

	struct ModelViewProjectionBlock
	{
        Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
	};
    
	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();
        
        if (m_AutoSpin) {
            UpdateAnim(time, delta);
        }

        UpdateFPS(time, delta);

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		m_MVPData.view = m_ViewCamera.GetView();
		m_MVPData.projection = m_ViewCamera.GetProjection();
        
        m_RoleMaterial->BeginFrame();
        for (int32 j = 0; j < m_RoleModel->meshes.size(); ++j)
        {
            m_RoleMaterial->BeginObject();
            m_RoleMaterial->SetLocalUniform("uboMVP", &m_MVPData, sizeof(ModelViewProjectionBlock));
            m_RoleMaterial->EndObject();
        }
        m_RoleMaterial->EndFrame();
        
		SetupCommandBuffers(bufferIndex);
		DemoBase::Present(bufferIndex);
	}
    
    void UpdateAnim(float time, float delta)
    {
        m_MVPData.model.AppendRotation(45.0f * delta, Vector3::RightVector);
        m_MVPData.model.AppendRotation(60.0f * delta, Vector3::ForwardVector);
    }

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();
        
        vk_demo::DVKPrimitive* primitive = m_RoleModel->meshes[0]->primitives[0];
        
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("InstanceDrawDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            
            ImGui::Checkbox("AutoSpin", &m_AutoSpin);
            ImGui::SliderInt("Instance", &(primitive->indexBuffer->instanceCount), 1, INSTANCE_COUNT);
            
            ImGui::Text("DrawCall:1");
			ImGui::Text("Triangle:%d", primitive->triangleNum * primitive->indexBuffer->instanceCount);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);
        
        m_RoleTexture = vk_demo::DVKTexture::Create2D(
            "assets/models/LizardMage/Body_colors1.jpg",
            m_VulkanDevice,
            cmdBuffer
        );
        
        m_RoleShader = vk_demo::DVKShader::Create(
            m_VulkanDevice,
            true,
            "assets/shaders/33_InstanceDraw/obj.vert.spv",
            "assets/shaders/33_InstanceDraw/obj.frag.spv"
        );
        
        m_RoleMaterial = vk_demo::DVKMaterial::Create(
            m_VulkanDevice,
            m_RenderPass,
            m_PipelineCache,
            m_RoleShader
        );
        m_RoleMaterial->PreparePipeline();
        m_RoleMaterial->SetTexture("diffuseMap", m_RoleTexture);
        
        m_RoleModel = vk_demo::DVKModel::LoadFromFile(
            "assets/models/LizardMage/LizardMage_Lowpoly.obj",
            m_VulkanDevice,
            cmdBuffer,
            {
                VertexAttribute::VA_Position,
                VertexAttribute::VA_Normal,
                VertexAttribute::VA_UV0
            }
        );
        
        // instance data
        vk_demo::DVKMesh* mesh = m_RoleModel->meshes[0];
        Matrix4x4 meshGlobal = mesh->linkNode->GetGlobalMatrix();
        vk_demo::DVKPrimitive* primitive = m_RoleModel->meshes[0]->primitives[0];
        primitive->instanceDatas.resize(8 * INSTANCE_COUNT);
        
        for (int32 i = 0; i < INSTANCE_COUNT; ++i)
        {
            Vector3 translate;
            translate.x = MMath::RandRange(-100.0f, 100.0f);
            translate.y = MMath::RandRange(-100.0f, 100.0f);
            translate.z = MMath::RandRange(-100.0f, 100.0f);
            
            Matrix4x4 matrix = meshGlobal;
            matrix.AppendRotation(MMath::RandRange(0.0f, 360.0f), Vector3::UpVector);
            matrix.AppendTranslation(translate);
            
            Quat quat   = matrix.ToQuat();
            Vector3 pos = matrix.GetOrigin();
            float dx = (+0.5) * ( pos.x * quat.w + pos.y * quat.z - pos.z * quat.y);
            float dy = (+0.5) * (-pos.x * quat.z + pos.y * quat.w + pos.z * quat.x);
            float dz = (+0.5) * ( pos.x * quat.y - pos.y * quat.x + pos.z * quat.w);
            float dw = (-0.5) * ( pos.x * quat.x + pos.y * quat.y + pos.z * quat.z);

            int32 index = i * 8;
            primitive->instanceDatas[index + 0] = quat.x;
            primitive->instanceDatas[index + 1] = quat.y;
            primitive->instanceDatas[index + 2] = quat.z;
            primitive->instanceDatas[index + 3] = quat.w;
            primitive->instanceDatas[index + 4] = dx;
            primitive->instanceDatas[index + 5] = dy;
            primitive->instanceDatas[index + 6] = dz;
            primitive->instanceDatas[index + 7] = dw;
        }
        
        primitive->indexBuffer->instanceCount = INSTANCE_COUNT;
        primitive->instanceBuffer = vk_demo::DVKVertexBuffer::Create(m_VulkanDevice, cmdBuffer, primitive->instanceDatas, m_RoleShader->instancesAttributes);
        
		delete cmdBuffer;
	}

	void DestroyAssets()
	{
        delete m_RoleModel;
        delete m_RoleShader;
        delete m_RoleMaterial;
        delete m_RoleTexture;
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		VkViewport viewport = {};
		viewport.x        = 0;
		viewport.y        = m_FrameHeight;
		viewport.width    = m_FrameWidth;
		viewport.height   = -(float)m_FrameHeight;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = m_FrameWidth;
		scissor.extent.height = m_FrameHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		// second pass
		{
			VkClearValue clearValues[2];
			clearValues[0].color        = { { 0.2f, 0.2f, 0.2f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };

			VkRenderPassBeginInfo renderPassBeginInfo;
			ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
			renderPassBeginInfo.renderPass               = m_RenderPass;
			renderPassBeginInfo.framebuffer              = m_FrameBuffers[backBufferIndex];
			renderPassBeginInfo.clearValueCount          = 2;
			renderPassBeginInfo.pClearValues             = clearValues;
			renderPassBeginInfo.renderArea.offset.x      = 0;
			renderPassBeginInfo.renderArea.offset.y      = 0;
			renderPassBeginInfo.renderArea.extent.width  = m_FrameWidth;
			renderPassBeginInfo.renderArea.extent.height = m_FrameHeight;
			vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);
            
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_RoleMaterial->GetPipeline());
			for (int32 i = 0; i < m_RoleModel->meshes.size(); ++i)
			{
                m_RoleMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, i);
                m_RoleModel->meshes[i]->BindDrawCmd(commandBuffer);
			}

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
        vk_demo::DVKBoundingBox bounds = m_RoleModel->rootNode->GetBounds();
        Vector3 boundSize   = bounds.max - bounds.min;
        Vector3 boundCenter = bounds.min + boundSize * 0.5f;
        boundCenter.z -= boundSize.Size() * 20.0f;
        
		m_ViewCamera.SetPosition(boundCenter);
		m_ViewCamera.Perspective(PI / 4, GetWidth(), GetHeight(), 0.1f, 1000.0f);
	}

	void CreateGUI()
	{
		m_GUI = new ImageGUIContext();
		m_GUI->Init("assets/fonts/Ubuntu-Regular.ttf");
	}

	void DestroyGUI()
	{
		m_GUI->Destroy();
		delete m_GUI;
	}

private:
    
	bool 						m_Ready = false;
	vk_demo::DVKCamera			m_ViewCamera;

	ModelViewProjectionBlock	m_MVPData;

	vk_demo::DVKModel*			m_RoleModel = nullptr;
	vk_demo::DVKShader*			m_RoleShader = nullptr;
    vk_demo::DVKMaterial*       m_RoleMaterial = nullptr;
    vk_demo::DVKTexture*        m_RoleTexture = nullptr;
    
    bool                        m_AutoSpin = false;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<InstanceDrawDemo>(1400, 900, "InstanceDrawDemo", cmdLine);
}
