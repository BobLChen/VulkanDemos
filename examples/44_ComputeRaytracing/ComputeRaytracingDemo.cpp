#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include "Loader/ImageLoader.h"

#include <vector>

class ComputeRaytracingDemo : public DemoBase
{
public:
	ComputeRaytracingDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~ComputeRaytracingDemo()
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

		CreateGUI();
		InitParmas();
		LoadAssets();
		ProcessRaytracing();

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

	struct Sphere
	{
		Vector3 position;
		float	radius;
		Vector3	diffuse;
		float	specular;
		uint32	id;
		Vector3	padding;
	};

	struct Plane
	{
		Vector3 normal;
		float   distance;
		Vector3 diffuse;
		float   specular;
		uint32  id;
		Vector3 padding;
	};

	struct RaytracingParamBlock
	{
		Sphere		spheres[3];
		Plane		planes[6];

		Vector3		lightPos;
		float		padding;

		Vector4		fogColor;

		Vector3		cameraPos;
		float		aspect;

		Matrix4x4	invProjection;
		Matrix4x4	invView;
	};
	
	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);
		UpdateUI(time, delta);

		SetupGfxCommand(bufferIndex);

		DemoBase::Present(bufferIndex);
	}
    
	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("ComputeRaytracingDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
            
			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	uint32 m_ID = 0;

	Sphere NewSphere(const Vector3& position, float radius, const Vector3& diffuse, float specular)
	{
		Sphere sphere;
		sphere.id       = m_ID++;
		sphere.position = position;
		sphere.radius   = radius;
		sphere.diffuse  = diffuse;
		sphere.specular = specular;
		return sphere;
	}

	Plane NewPlane(const Vector3& normal, float distance, const Vector3& diffuse, float specular)
	{
		Plane plane;
		plane.id       = m_ID++;
		plane.normal   = normal;
		plane.distance = distance;
		plane.diffuse  = diffuse;
		plane.specular = specular;
		return plane;
	}

	void ProcessRaytracing()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(
			m_VulkanDevice, 
			m_ComputeCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			m_VulkanDevice->GetComputeQueue()
		);

		// create target image
        m_ComputeTarget = vk_demo::DVKTexture::Create2D(
            m_VulkanDevice,
            cmdBuffer,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_ASPECT_COLOR_BIT,
            1024, 1024,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_SAMPLE_COUNT_1_BIT,
            ImageLayoutBarrier::ComputeGeneralRW
        );
        
        m_ComputeShader    = vk_demo::DVKShader::Create(
            m_VulkanDevice,
            "assets/shaders/44_ComputeRaytracing/Raytracing.comp.spv"
        );
        m_ComputeProcessor = vk_demo::DVKCompute::Create(m_VulkanDevice, m_PipelineCache, m_ComputeShader);
        m_ComputeProcessor->SetStorageTexture("outputImage", m_ComputeTarget);
		m_ComputeProcessor->SetStorageBuffer("inSceneData", m_SceneBuffer);
		m_ComputeProcessor->SetUniform("uboParam", &m_RaytracingParam, sizeof(RaytracingParamBlock));

		// compute command
		cmdBuffer->Begin();

        m_ComputeProcessor->BindDispatch(cmdBuffer->cmdBuffer, m_ComputeTarget->width / 16, m_ComputeTarget->height / 16, 1);
		
		cmdBuffer->End();
		cmdBuffer->Submit();
        
        m_Material->SetTexture("diffuseMap", m_ComputeTarget);
        
        delete cmdBuffer;
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Quad = vk_demo::DVKDefaultRes::fullQuad;

		m_Shader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/44_ComputeRaytracing/Texture.vert.spv",
			"assets/shaders/44_ComputeRaytracing/Texture.frag.spv"
		);

		m_Material = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader
		);
		m_Material->pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_NONE;
		m_Material->PreparePipeline();

		// prepare scene data
		m_SceneModel = vk_demo::DVKModel::LoadFromFile(
			"assets/models/simplescene.obj",
			m_VulkanDevice,
			nullptr,
			{ 
				VertexAttribute::VA_Position
			}
		);
        
		// copy scene data to storage buffer
		int32 count = 0;
		std::vector<float> bufferDatas;
		// vec4 datas[0] = count
		bufferDatas.push_back(0);
		bufferDatas.push_back(0);
		bufferDatas.push_back(0);
		bufferDatas.push_back(0);

		for (int32 meshID = 0; meshID < m_SceneModel->meshes.size(); ++meshID)
		{
			auto mesh = m_SceneModel->meshes[meshID];
			for (int32 primitiveID = 0; primitiveID < mesh->primitives.size(); ++primitiveID)
			{
				count += 1;

				auto primitive = mesh->primitives[primitiveID];
				
				bufferDatas.push_back(meshID);
				bufferDatas.push_back(primitiveID);
				bufferDatas.push_back(primitive->vertexCount);
				bufferDatas.push_back(primitive->triangleNum);
				
				for (int32 i = 0; i < primitive->triangleNum; ++i) {
					bufferDatas.push_back(primitive->indices[i * 3 + 0]);
					bufferDatas.push_back(primitive->indices[i * 3 + 1]);
					bufferDatas.push_back(primitive->indices[i * 3 + 2]);
					bufferDatas.push_back(0);
				}
				for (int32 i = 0; i < primitive->vertexCount; ++i) {
					bufferDatas.push_back(primitive->vertices[i * 3 + 0]);
					bufferDatas.push_back(primitive->vertices[i * 3 + 1]);
					bufferDatas.push_back(primitive->vertices[i * 3 + 2]);
					bufferDatas.push_back(0);
				}
			}
		}
		bufferDatas[0] = count;

		vk_demo::DVKBuffer* stagingBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			bufferDatas.size() * sizeof(float), 
			bufferDatas.data()
		);

		m_SceneBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			bufferDatas.size() * sizeof(float)
		);

		cmdBuffer->Begin();

		VkBufferCopy copyRegion = {};
		copyRegion.size = bufferDatas.size() * sizeof(float);
		vkCmdCopyBuffer(cmdBuffer->cmdBuffer, stagingBuffer->buffer, m_SceneBuffer->buffer, 1, &copyRegion);

		cmdBuffer->End();
		cmdBuffer->Submit();

		delete stagingBuffer;

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
        delete m_ComputeShader;
        delete m_ComputeTarget;
        delete m_ComputeProcessor;
        
		delete m_Material;
		delete m_Shader;

		delete m_SceneModel;
		delete m_SceneBuffer;
	}

	void SetupGfxCommand(int32 backBufferIndex)
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

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());

		m_Material->BeginFrame();
		m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		m_Quad->meshes[0]->BindDrawCmd(commandBuffer);
		m_Material->EndFrame();

		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);
		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		vk_demo::DVKCamera camera;
		camera.SetPosition(0, 2.5f, -10.0f);
		camera.LookAt(0, 2.5f, 0);
		camera.Perspective(PI / 4, 1.0f, 1.0f, 1.0f, 1500.0f);
        
		m_RaytracingParam.invView = camera.GetView();
		m_RaytracingParam.invView.SetInverse();
		m_RaytracingParam.invProjection = camera.GetProjection();
		m_RaytracingParam.invProjection.SetInverse();

		m_RaytracingParam.lightPos.x = 0.0f;
		m_RaytracingParam.lightPos.y = 5.0f;
		m_RaytracingParam.lightPos.z = 0.0f;
        
		m_RaytracingParam.fogColor.x = 0.0f;
		m_RaytracingParam.fogColor.y = 0.0f;
		m_RaytracingParam.fogColor.z = 0.0f;

        m_RaytracingParam.cameraPos = camera.GetTransform().GetOrigin();

		m_RaytracingParam.aspect    = GetWidth() * 1.0f / GetHeight();
		
		m_RaytracingParam.spheres[0] = NewSphere(Vector3( 1.75f, -0.5f,   0.0f), 1.0f, Vector3(0.00f, 1.00f, 0.00f), 32.0f);
		m_RaytracingParam.spheres[1] = NewSphere(Vector3( 0.0f,   1.0f,   0.5f), 1.0f, Vector3(0.65f, 0.77f, 0.97f), 32.0f);
		m_RaytracingParam.spheres[2] = NewSphere(Vector3(-1.75f, -0.75f,  0.5f), 1.0f, Vector3(0.90f, 0.76f, 0.46f), 32.0f);

		m_RaytracingParam.planes[0]  = NewPlane(Vector3( 0.0f,  1.0f,  0.0f), 4.0f, Vector3(1.0f, 1.0f, 1.0f), 32.0f);
		m_RaytracingParam.planes[1]  = NewPlane(Vector3( 0.0f, -1.0f,  0.0f), 4.0f, Vector3(1.0f, 1.0f, 1.0f), 32.0f);
		m_RaytracingParam.planes[2]  = NewPlane(Vector3( 0.0f,  0.0f,  1.0f), 4.0f, Vector3(1.0f, 1.0f, 1.0f), 32.0f);
		m_RaytracingParam.planes[3]  = NewPlane(Vector3( 0.0f,  0.0f, -1.0f), 4.0f, Vector3(0.0f, 0.0f, 0.0f), 32.0f);
		m_RaytracingParam.planes[4]  = NewPlane(Vector3(-1.0f,  0.0f,  0.0f), 4.0f, Vector3(1.0f, 0.0f, 0.0f), 32.0f);
		m_RaytracingParam.planes[5]  = NewPlane(Vector3( 1.0f,  0.0f,  0.0f), 4.0f, Vector3(0.0f, 1.0f, 0.0f), 32.0f);
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

	bool 						    m_Ready = false;

	vk_demo::DVKBuffer*				m_SceneBuffer = nullptr;
	vk_demo::DVKModel*				m_SceneModel = nullptr;

	vk_demo::DVKModel*				m_Quad = nullptr;
	vk_demo::DVKMaterial*		    m_Material = nullptr;
	vk_demo::DVKShader*			    m_Shader = nullptr;

    vk_demo::DVKTexture*            m_ComputeTarget = nullptr;
    vk_demo::DVKShader*             m_ComputeShader = nullptr;
    vk_demo::DVKCompute*            m_ComputeProcessor = nullptr;

	RaytracingParamBlock			m_RaytracingParam;
    
	ImageGUIContext*			    m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<ComputeRaytracingDemo>(1400, 900, "ComputeRaytracingDemo", cmdLine);
}
