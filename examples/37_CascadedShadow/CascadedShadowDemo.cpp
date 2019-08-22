#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"
#include "Demo/DVKTexture.h"
#include "Demo/DVKRenderTarget.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"
#include "File/FileManager.h"
#include "UI/ImageGUIContext.h"

#include <vector>
#include <fstream>

class CascadedShadowDemo : public DemoBase
{
public:
	CascadedShadowDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~CascadedShadowDemo()
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

		CreateRenderTarget();
		CreateGUI();
		LoadAssets();
		InitParmas();

		m_Ready = true;

		return true;
	}

	virtual void Exist() override
	{
		DemoBase::Release();

		DestroyRenderTarget();
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

	struct DirectionalLightBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 projection;
		Vector4 direction;
	};

	struct ShadowParamBlock
	{
		Vector4 bias;
	};

	void UpdateLight(float time, float delta)
	{
		if (!m_AnimLight) {
			return;
		}
		m_LightCamera.view.SetIdentity();
		m_LightCamera.view.SetOrigin(Vector3(50.0f * MMath::Sin(time), 80.0f, 50.0f * MMath::Cos(time)));
		m_LightCamera.view.LookAt(Vector3(0, 0, 0));
		m_LightCamera.direction = -m_LightCamera.view.GetForward().GetSafeNormal();
		m_LightCamera.view.SetInverse();
	}
    
    void UpdateDepthMaterial()
    {
        // depth
        m_DepthMaterial->BeginFrame();
        
        // ground
        m_LightCamera.model = m_GroundModel->meshes[0]->linkNode->GetGlobalMatrix();
        m_DepthMaterial->BeginObject();
        m_DepthMaterial->SetLocalUniform("uboMVP", &m_LightCamera, sizeof(DirectionalLightBlock));
        m_DepthMaterial->EndObject();
        
        // Torus
        m_LightCamera.model = m_TorusModel->meshes[0]->linkNode->GetGlobalMatrix();
        m_DepthMaterial->BeginObject();
        m_DepthMaterial->SetLocalUniform("uboMVP", &m_LightCamera, sizeof(DirectionalLightBlock));
        m_DepthMaterial->EndObject();
        
        m_DepthMaterial->EndFrame();
    }
    
    void UpdateShadeMaterial()
    {
        // shade
        vk_demo::DVKMaterial* shadowMaterial = m_ShadowList[m_Selected];
        shadowMaterial->BeginFrame();
        
        // ground
        m_MVPData.model = m_GroundModel->meshes[0]->linkNode->GetGlobalMatrix();
        shadowMaterial->BeginObject();
        shadowMaterial->SetLocalUniform("uboMVP",      &m_MVPData,      sizeof(ModelViewProjectionBlock));
        shadowMaterial->SetLocalUniform("lightMVP",    &m_LightCamera,  sizeof(DirectionalLightBlock));
        shadowMaterial->SetLocalUniform("shadowParam", &m_ShadowParam,  sizeof(ShadowParamBlock));
        shadowMaterial->EndObject();
        
        // torus
        m_MVPData.model = m_TorusModel->meshes[0]->linkNode->GetGlobalMatrix();
        shadowMaterial->BeginObject();
        shadowMaterial->SetLocalUniform("uboMVP",      &m_MVPData,      sizeof(ModelViewProjectionBlock));
        shadowMaterial->SetLocalUniform("lightMVP",    &m_LightCamera,  sizeof(DirectionalLightBlock));
        shadowMaterial->SetLocalUniform("shadowParam", &m_ShadowParam,  sizeof(ShadowParamBlock));
        shadowMaterial->EndObject();
        
        shadowMaterial->EndFrame();
    }
    
	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);
		UpdateUI(time, delta);
		UpdateLight(time, delta);
        UpdateDepthMaterial();
        UpdateShadeMaterial();
        
		SetupCommandBuffers(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	void UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("CascadedShadowDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Checkbox("Auto Spin", &m_AnimLight);

			ImGui::Combo("Shadow", &m_Selected, m_ShadowNames.data(), m_ShadowNames.size());

			ImGui::SliderFloat("Bias", &m_ShadowParam.bias.x, 0.0f, 0.05f, "%.4f");

			if (m_Selected != 0) 
			{
				ImGui::SliderFloat("Step", &m_ShadowParam.bias.y, 0.0f, 10.0f);
			}
			
			ImGui::Text("ShadowMap:%dx%d", m_ShadowMap->width, m_ShadowMap->height);
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		m_GUI->EndFrame();
		m_GUI->Update();
	}

	void CreateRenderTarget()
	{
		m_ShadowMap = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			PixelFormatToVkFormat(m_DepthFormat, false),
			VK_IMAGE_ASPECT_DEPTH_BIT,
			2048, 2048,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		vk_demo::DVKRenderPassInfo passInfo(m_ShadowMap, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
		m_ShadowRTT = vk_demo::DVKRenderTarget::Create(m_VulkanDevice, passInfo);
	}

	void DestroyRenderTarget()
	{
		delete m_ShadowRTT;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Quad = vk_demo::DVKDefaultRes::fullQuad;
        
		m_TorusModel = vk_demo::DVKModel::LoadFromFile(
			"assets/models/torus.obj",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position,
				VertexAttribute::VA_Normal
			}
		);
        
        m_GroundModel = vk_demo::DVKModel::LoadFromFile(
            "assets/models/plane.obj",
            m_VulkanDevice,
            cmdBuffer,
            {
                VertexAttribute::VA_Position,
                VertexAttribute::VA_Normal
            }
        );
        m_GroundModel->rootNode->localMatrix.AppendScale(Vector3(100, 100, 100));
        
		// depth
		m_DepthShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/37_CascadedShadow/Depth.vert.spv",
			"assets/shaders/37_CascadedShadow/Depth.frag.spv"
		);

		m_DepthMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_ShadowRTT,
			m_PipelineCache,
			m_DepthShader
		);
		m_DepthMaterial->pipelineInfo.colorAttachmentCount = 0;
		m_DepthMaterial->PreparePipeline();

		// simple shadow
		m_SimpleShadowShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/37_CascadedShadow/SimpleShadow.vert.spv",
			"assets/shaders/37_CascadedShadow/SimpleShadow.frag.spv"
		);

		m_SimpleShadowMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_SimpleShadowShader
		);
		m_SimpleShadowMaterial->PreparePipeline();
		m_SimpleShadowMaterial->SetTexture("shadowMap", m_ShadowMap);

		// pcf shadow
		m_PCFShadowShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/37_CascadedShadow/PCFShadow.vert.spv",
			"assets/shaders/37_CascadedShadow/PCFShadow.frag.spv"
		);

		m_PCFShadowMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_PCFShadowShader
		);
		m_PCFShadowMaterial->PreparePipeline();
		m_PCFShadowMaterial->SetTexture("shadowMap", m_ShadowMap);

		// ui used
		m_ShadowNames.push_back("Simple");
		m_ShadowNames.push_back("PCF");

		m_ShadowList.push_back(m_SimpleShadowMaterial);
		m_ShadowList.push_back(m_PCFShadowMaterial);

		// debug
		m_DebugShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/37_CascadedShadow/Debug.vert.spv",
			"assets/shaders/37_CascadedShadow/Debug.frag.spv"
		);

		m_DebugMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_DebugShader
		);

		m_DebugMaterial->PreparePipeline();
		m_DebugMaterial->SetTexture("depthTexture", m_ShadowMap);

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_TorusModel;
        delete m_GroundModel;

		delete m_DepthShader;
		delete m_DepthMaterial;

		delete m_DebugMaterial;
		delete m_DebugShader;

		delete m_ShadowMap;

		delete m_SimpleShadowShader;
		delete m_SimpleShadowMaterial;

		delete m_PCFShadowShader;
		delete m_PCFShadowMaterial;
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

		// render target pass
		{
			m_ShadowRTT->BeginRenderPass(commandBuffer);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DepthMaterial->GetPipeline());
			
            // ground
            m_DepthMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
            m_GroundModel->meshes[0]->BindDrawCmd(commandBuffer);
            
            // trus
            m_DepthMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 1);
            m_TorusModel->meshes[0]->BindDrawCmd(commandBuffer);
            
			m_ShadowRTT->EndRenderPass(commandBuffer);
		}

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

			// shade
			vk_demo::DVKMaterial* shadowMaterial = m_ShadowList[m_Selected];
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMaterial->GetPipeline());
			
            // ground
            shadowMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
            m_GroundModel->meshes[0]->BindDrawCmd(commandBuffer);
            
            // shade
            shadowMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 1);
            m_TorusModel->meshes[0]->BindDrawCmd(commandBuffer);
            
			// debug
			viewport.x = m_FrameWidth * 0.75f;
			viewport.y = m_FrameHeight * 0.25f;
			viewport.width  = m_FrameWidth * 0.25f;
			viewport.height = -(float)m_FrameHeight * 0.25f;    // flip y axis

			scissor.offset.x = m_FrameWidth * 0.75f;
			scissor.offset.y = 0;
			scissor.extent.width  = m_FrameWidth  * 0.25f;
			scissor.extent.height = m_FrameHeight * 0.25f;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DebugMaterial->GetPipeline());
			m_DebugMaterial->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
			m_Quad->meshes[0]->BindDrawCmd(commandBuffer);

			m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

			vkCmdEndRenderPass(commandBuffer);
		}

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		// model view projection
		m_MVPData.model.SetIdentity();

		m_MVPData.view.SetIdentity();
		m_MVPData.view.SetOrigin(Vector3(0.0f, 300.0f, -500.0f));
		m_MVPData.view.LookAt(Vector3(0, 0, 0));
		m_MVPData.view.SetInverse();

		m_MVPData.projection.SetIdentity();
		m_MVPData.projection.Perspective(MMath::DegreesToRadians(75.0f), (float)GetWidth(), (float)GetHeight(), 1.0f, 1500.0f);

		// light camera
		m_LightCamera.view.SetIdentity();
		m_LightCamera.view.SetOrigin(Vector3(-700.0f, 400.0f, 0.0f));
		m_LightCamera.view.LookAt(Vector3(0, 0, 0));
		m_LightCamera.direction = -m_LightCamera.view.GetForward().GetSafeNormal();
		m_LightCamera.view.SetInverse();
        
        int32 size = 512;
		m_LightCamera.projection.SetIdentity();
		m_LightCamera.projection.Orthographic(-size, size, -size, size, 1.0f, 3000.0f);

		// shadow bias
		m_ShadowParam.bias.x = 0.005f;
		m_ShadowParam.bias.y = 5.0f;
		m_ShadowParam.bias.z = 0.0f;
		m_ShadowParam.bias.w = 0.0f;

		UpdateCascadeShadow();
	}

	void UpdateCascadeShadow()
	{
		Matrix4x4 invProjection = m_MVPData.projection.Inverse();
		Matrix4x4 invModelview  = m_MVPData.view.Inverse();

		Vector3 direction = Vector3(0, 0, 1);
		Vector3 side = Vector3::CrossProduct(Vector3(0.0f, 0.0f, 1.0f), direction);
		Vector3 up   = Vector3::CrossProduct(direction, side);
		
		Vector3 points[8] = {
			Vector3(-1.0f,-1.0f,-1.0f), Vector3(1.0f,-1.0f,-1.0f), Vector3(-1.0f,1.0f,-1.0f), Vector3(1.0f,1.0f,-1.0f),
			Vector3(-1.0f,-1.0f, 1.0f), Vector3(1.0f,-1.0f, 1.0f), Vector3(-1.0f,1.0f, 1.0f), Vector3(1.0f,1.0f, 1.0f),
		};

		for(int32 i = 0; i < 8; i++) {
			Vector4 point = invProjection.TransformVector4(Vector4(points[i]));
			points[i] = Vector3(point) / point.w;
		}

		Vector3 directions[4];
		for(int32 i = 0; i < 4; i++) {
			directions[i] = (points[i + 4] - points[i]).GetSafeNormal();
		}

		float zNear = 0.1f;
		float zFar  = 1000.0f;
		float shadowRange = 2000.0f;
		float shadowDistribute = 0.25f;

		for (int32 i = 0; i < 4; ++i)
		{
			float k0   = (float)(i + 0) / 4;
			float k1   = (float)(i + 1) / 4;
			float fmin = MMath::Lerp(zNear * powf(zFar / zNear,k0), zNear + (zFar - zNear) * k0, shadowDistribute);
			float fmax = MMath::Lerp(zNear * powf(zFar / zNear,k1), zNear + (zFar - zNear) * k1, shadowDistribute);

			Vector3 mmin(1000);
			Vector3 mmax(-1000);
			for(int j = 0; j < 4; j++) {
				Vector3 tmin = points[j] + directions[j] * fmin;
				Vector3 tmax = points[j] + directions[j] * fmax;
				if (mmin.x > tmin.x) mmin.x = tmin.x;
				if (mmax.x < tmin.x) mmax.x = tmin.x;
				if (mmin.y > tmin.y) mmin.y = tmin.y;
				if (mmax.y < tmin.y) mmax.y = tmin.y;
				if (mmin.z > tmin.z) mmin.z = tmin.z;
				if (mmax.z < tmin.z) mmax.z = tmin.z;

				if (mmin.x > tmax.x) mmin.x = tmax.x;
				if (mmax.x < tmax.x) mmax.x = tmax.x;
				if (mmin.y > tmax.y) mmin.y = tmax.y;
				if (mmax.y < tmax.y) mmax.y = tmax.y;
				if (mmin.z > tmax.z) mmin.z = tmax.z;
				if (mmax.z < tmax.z) mmax.z = tmax.z;
			}

			Vector3 extend = mmax - mmin;
			Vector3 center = mmin + extend * 0.5f;
			
			float halfSize = 512 / 2.0f;
			Vector3 target = invModelview.TransformVector4(center);
			float x = MMath::CeilToFloat(Vector3::DotProduct(target, up)   * halfSize / extend.Size()) * extend.Size() / halfSize;
			float y = MMath::CeilToFloat(Vector3::DotProduct(target, side) * halfSize / extend.Size()) * extend.Size() / halfSize;
			target = up * x + side * y + direction * Vector3::DotProduct(target,direction);
			
			MLOG("");
			//projections[i] = ortho(bs.getRadius(),-bs.getRadius(),bs.getRadius(),-bs.getRadius(),shadow_range / 1000.0f,shadow_range);
			//modelviews[i] = lookAt(target + direction * shadow_range / 2.0f,target - direction * shadow_range / 2.0f,up);
		}
	}

	void CreateGUI()
	{
		m_GUI = new ImageGUIContext();
		m_GUI->Init("assets/fonts/Roboto-Medium.ttf");
	}

	void DestroyGUI()
	{
		m_GUI->Destroy();
		delete m_GUI;
	}

private:

	typedef std::vector<vk_demo::DVKTexture*>			TextureArray;
	typedef std::vector<vk_demo::DVKMaterial*>			MaterialArray;
	typedef std::vector<std::vector<vk_demo::DVKMesh*>> MatMeshArray;

	bool 						m_Ready = false;

	// Debug
	vk_demo::DVKModel*			m_Quad = nullptr;
	vk_demo::DVKMaterial*	    m_DebugMaterial;
	vk_demo::DVKShader*		    m_DebugShader;

	// Shadow Rendertarget
	vk_demo::DVKRenderTarget*   m_ShadowRTT = nullptr;
	vk_demo::DVKTexture*        m_ShadowMap = nullptr;

	// depth 
	vk_demo::DVKShader*			m_DepthShader = nullptr;
	vk_demo::DVKMaterial*		m_DepthMaterial = nullptr;

	// mvp
	ModelViewProjectionBlock	m_MVPData;
	vk_demo::DVKModel*			m_TorusModel = nullptr;
    vk_demo::DVKModel*          m_GroundModel = nullptr;
    
	// light
	DirectionalLightBlock		m_LightCamera;
	ShadowParamBlock			m_ShadowParam;

	// obj render
	vk_demo::DVKShader*			m_SimpleShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_SimpleShadowMaterial = nullptr;

	vk_demo::DVKShader*			m_PCFShadowShader = nullptr;
	vk_demo::DVKMaterial*		m_PCFShadowMaterial = nullptr;

	bool                        m_AnimLight = true;
	int32						m_Selected = 1;
	std::vector<const char*>	m_ShadowNames;
	MaterialArray				m_ShadowList;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<CascadedShadowDemo>(1400, 900, "CascadedShadowDemo", cmdLine);
}
