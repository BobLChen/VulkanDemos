#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

// less than m_VulkanDevice->GetLimits().maxUniformBufferRange
#define INSTANCE_COUNT 512

struct InstanceData
{
	Matrix4x4	transforms[INSTANCE_COUNT];
	Vector4		colors[INSTANCE_COUNT];
};

struct ParticleData
{
	Vector3 position;
	Vector3 velocity;
	Vector3 direction;
	float	grivity;
	float   time;
	float   lifeTime;
};

struct ModelViewProjectionBlock
{
	Matrix4x4 model;
	Matrix4x4 view;
	Matrix4x4 proj;
};

std::mutex writeMutex;

class ParticleModel
{
public:
	ParticleModel(vk_demo::DVKModel* model, vk_demo::DVKMaterial* material, vk_demo::DVKModel* templat, int32 baseIndex, int32 count)
		: m_Model(model)
		, m_Material(material)
		, m_Template(templat)
		, m_BaseIndex(baseIndex)
		, m_Count(count)
		, m_UpdateIndex(0)
	{

	}

	void Draw(VkCommandBuffer commandBuffer, vk_demo::DVKCamera& camera)
	{
		vk_demo::DVKPrimitive* primitive = m_Model->meshes[0]->primitives[0];

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());

		m_MVPParam.model = m_Model->meshes[0]->linkNode->GetGlobalMatrix();
		m_MVPParam.view  = camera.GetView();
		m_MVPParam.proj  = camera.GetProjection();

		{
			std::lock_guard<std::mutex> lockGuard(writeMutex);

			m_Material->BeginFrame();

			m_Material->BeginObject();
			m_Material->SetLocalUniform("uboMVP",		&m_MVPParam,		sizeof(ModelViewProjectionBlock));
			m_Material->SetLocalUniform("uboTransform", &m_InstanceData,	sizeof(InstanceData));
			m_Material->EndObject();

			m_Material->EndFrame();

			m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);
		}
		
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(primitive->vertexBuffer->dvkBuffer->buffer), &(primitive->vertexBuffer->offset));
		vkCmdBindVertexBuffers(commandBuffer, 1, 1, &(primitive->instanceBuffer->dvkBuffer->buffer), &(primitive->instanceBuffer->offset));
		vkCmdBindIndexBuffer(commandBuffer, primitive->indexBuffer->dvkBuffer->buffer, 0, primitive->indexBuffer->indexType);
		vkCmdDrawIndexed(commandBuffer, primitive->indexBuffer->indexCount, m_UpdateIndex, 0, 0, 0);
		
	}

	void Update(std::vector<Matrix4x4>& bonesData, vk_demo::DVKCamera& camera, float time, float delta)
	{
		// ring buffer
		if (m_UpdateIndex + m_Count > m_Model->meshes[0]->primitives[0]->indexBuffer->instanceCount) {
			m_UpdateIndex = 0;
		}

		// move particle
		for (int32 index = 0; index < m_UpdateIndex; ++index)
		{
			if (m_ParticleDatas[index].time >= m_ParticleDatas[index].lifeTime) {
				m_InstanceData.colors[index].w = 0;
				continue;
			}

			m_ParticleDatas[index].time       += delta;
			m_ParticleDatas[index].position   += m_ParticleDatas[index].direction * m_ParticleDatas[index].velocity * delta;
			m_ParticleDatas[index].position.y += m_ParticleDatas[index].grivity * delta;

			Matrix4x4 matrix;
			matrix.SetPosition(m_ParticleDatas[index].position);
			matrix.LookAt(camera.GetTransform().GetOrigin());
			
			m_InstanceData.colors[index].w   = m_ParticleDatas[index].time / m_ParticleDatas[index].lifeTime;
			m_InstanceData.transforms[index] = matrix;
		}

		// init particle
		vk_demo::DVKPrimitive* primitive = m_Template->meshes[0]->primitives[0];
		int32 stride    = primitive->vertices.size() / primitive->vertexCount;
		int32 vertBegin = m_BaseIndex * stride;
		int32 vertEnd   = (m_BaseIndex + m_Count) * stride;
		int32 objIndex  = m_UpdateIndex;

		for (int32 index = vertBegin; index < vertEnd; index += stride)
		{
			Vector3 position(
				primitive->vertices[index + 0],
				primitive->vertices[index + 1],
				primitive->vertices[index + 2]
			);
			IntVector4 skinIndices(
				primitive->vertices[index + 6],
				primitive->vertices[index + 7],
				primitive->vertices[index + 8],
				primitive->vertices[index + 9]
			);
			Vector4 skinWeights(
				primitive->vertices[index + 10],
				primitive->vertices[index + 11],
				primitive->vertices[index + 12],
				primitive->vertices[index + 13]
			);

			Vector3 finalPos = 
				bonesData[skinIndices.x].TransformPosition(position) * skinWeights.x + 
				bonesData[skinIndices.y].TransformPosition(position) * skinWeights.y + 
				bonesData[skinIndices.z].TransformPosition(position) * skinWeights.z + 
				bonesData[skinIndices.w].TransformPosition(position) * skinWeights.w;

			Matrix4x4 matrix;
			matrix.SetPosition(finalPos);
			matrix.LookAt(camera.GetTransform().GetOrigin());

			m_InstanceData.colors[objIndex]     = Vector4(MMath::FRandRange(0, 1.0f), MMath::FRandRange(0, 1.0f), MMath::FRandRange(0, 1.0f), 1.0f);
			m_InstanceData.transforms[objIndex] = matrix;

			m_ParticleDatas[objIndex].position  = finalPos;
			m_ParticleDatas[objIndex].direction = Vector3(MMath::FRandRange(0, 1.0f), MMath::FRandRange(0, 1.0f), MMath::FRandRange(0, 1.0f)).GetSafeNormal();
			m_ParticleDatas[objIndex].velocity  = Vector3(MMath::FRandRange(0, 1.0f), MMath::FRandRange(0, 1.0f), MMath::FRandRange(0, 1.0f)).GetSafeNormal() * MMath::FRandRange(5.0f, 15.0f);
			m_ParticleDatas[objIndex].grivity   = MMath::FRandRange(0, -5.0f);
			m_ParticleDatas[objIndex].lifeTime  = MMath::FRandRange(0.25f, 0.50f);
			m_ParticleDatas[objIndex].time      = 0;

			objIndex += 1;
		}

		m_UpdateIndex += m_Count;
	}

private:

	vk_demo::DVKModel*			m_Template;
	vk_demo::DVKModel*			m_Model;
	vk_demo::DVKMaterial*		m_Material;
	int32						m_BaseIndex;
	int32						m_Count;
	int32						m_UpdateIndex;
	InstanceData				m_InstanceData;
	ParticleData				m_ParticleDatas[INSTANCE_COUNT];
	ModelViewProjectionBlock	m_MVPParam;
};

struct ThreadData
{
	int32 index;
	int32 frameID;
	VkCommandPool commandPool;
	std::vector<ParticleModel*> particles;
	std::vector<vk_demo::DVKCommandBuffer*> threadCommandBuffers;
};

class MyThread
{
public:
	typedef std::function<void ()> ThreadFunc;

	explicit MyThread(ThreadFunc func)
		: m_ThreadFunc(func)
		, m_Thread(func)
	{

	}

	~MyThread()
	{
		if (m_Thread.joinable()) {
			m_Thread.join();
		}
	}

	MyThread(MyThread const&)=delete;

	MyThread& operator=(MyThread const&)=delete;

private:
	std::thread m_Thread;
	ThreadFunc  m_ThreadFunc;
};

class ThreadedRenderingDemo : public DemoBase
{
public:
	ThreadedRenderingDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
		
	}

	virtual ~ThreadedRenderingDemo()
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
		LoadAnimModel();
		LoadAssets();
		InitParmas();
		InitThreads();

		m_Ready = true;
		return true;
	}

	virtual void Exist() override
	{
		DestroyAssets();
		DestroyGUI();
		DemoBase::Release();
	}

	virtual void Loop(float time, float delta) override
	{
		if (!m_Ready) {
			return;
		}
		Draw(time, delta);
	}

private:

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		m_bufferIndex = bufferIndex;
		m_FrameTime   = time;
		m_FrameDelta  = delta;

		UpdateFPS(time, delta);

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		UpdateAnimation(time, delta);

		// notify fram start
		{
			std::lock_guard<std::mutex> lockGuard(m_FrameStartLock);
			m_ThreadDoneCount  = 0;
			m_MainFrameID     += 1;
			m_FrameStartCV.notify_all();
		}

		// wait for thread done
		{
			std::unique_lock<std::mutex> lockGuard(m_ThreadDoneLock);
			while (m_ThreadDoneCount != m_Threads.size()) {
				m_ThreadDoneCV.wait(lockGuard);
			}
		}

		SetupCommandBuffers(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("ThreadedRenderingDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void UpdateAnimation(float time, float delta)
	{
		m_RoleModel->Update(time, delta);

		vk_demo::DVKMesh* mesh = m_RoleModel->meshes[0];
		for (int32 i = 0; i < mesh->bones.size(); ++i)
		{
			int32 index = mesh->bones[i];
			m_BonesData[index] = m_RoleModel->bones[index]->finalTransform;
		}
	}

	void LoadAnimModel()
	{
		m_RoleModel = vk_demo::DVKModel::LoadFromFile(
			"assets/models/xiaonan/nvhai.fbx",
			m_VulkanDevice,
			nullptr,
			{
				VertexAttribute::VA_Position,
				VertexAttribute::VA_Normal,
				VertexAttribute::VA_SkinIndex,
				VertexAttribute::VA_SkinWeight
			}
		);

		m_RoleModel->SetAnimation(0);
		m_BonesData.resize(m_RoleModel->meshes[0]->bones.size());
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// fullscreen
		m_Quad = vk_demo::DVKDefaultRes::fullQuad;

		// scene model
		m_ParticleModel = vk_demo::DVKModel::LoadFromFile(
			"assets/models/plane_z.obj",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position,
				VertexAttribute::VA_UV0,
			}
		);

		m_ParticleShader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/54_ThreadedRendering/obj.vert.spv",
			"assets/shaders/54_ThreadedRendering/obj.frag.spv"
		);

		m_ParticleTexture = vk_demo::DVKTexture::Create2D(
			"assets/textures/flare3.png",
			m_VulkanDevice,
			cmdBuffer
		);

		m_ParticleMaterial = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_ParticleShader
		);
		m_ParticleMaterial->pipelineInfo.rasterizationState.cullMode = VK_CULL_MODE_NONE;
		m_ParticleMaterial->pipelineInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
		m_ParticleMaterial->pipelineInfo.inputAssemblyState.primitiveRestartEnable = VK_FALSE;
		m_ParticleMaterial->pipelineInfo.depthStencilState.depthTestEnable = VK_FALSE;
		m_ParticleMaterial->pipelineInfo.depthStencilState.depthWriteEnable = VK_FALSE;
		m_ParticleMaterial->pipelineInfo.depthStencilState.stencilTestEnable = VK_FALSE;
		m_ParticleMaterial->pipelineInfo.depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].blendEnable = VK_TRUE;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_ADD;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_ParticleMaterial->pipelineInfo.blendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		m_ParticleMaterial->PreparePipeline();
		m_ParticleMaterial->SetTexture("diffuseMap", m_ParticleTexture);
		
		// particle instance data
		{
			// write instance index
			vk_demo::DVKPrimitive* primitive = m_ParticleModel->meshes[0]->primitives[0];
			primitive->instanceDatas.resize(INSTANCE_COUNT);

			for (int32 i = 0; i < INSTANCE_COUNT; ++i) {
				primitive->instanceDatas[i] = i;
			}

			// create instance buffer
			primitive->indexBuffer->instanceCount = INSTANCE_COUNT;
			primitive->instanceBuffer = vk_demo::DVKVertexBuffer::Create(
				m_VulkanDevice, 
				cmdBuffer, 
				primitive->instanceDatas, 
				m_ParticleShader->instancesAttributes
			);
		}

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		m_ThreadRunning = false;
		vkQueueWaitIdle(m_VulkanDevice->GetPresentQueue()->GetHandle());
		m_FrameStartCV.notify_all();

		delete m_RoleModel;
		delete m_ParticleModel;
		delete m_ParticleShader;
		delete m_ParticleMaterial;
		delete m_ParticleTexture;

		for (int32 i = 0; i < m_Particles.size(); ++i) {
			delete m_Particles[i];
		}
		m_Particles.clear();

		for (int32 i = 0; i < m_UICommandBuffers.size(); ++i) {
			delete m_UICommandBuffers[i];
		}

		for (int32 i = 0; i < m_ThreadDatas.size(); ++i) 
		{
			for (int32 j = 0; j < m_ThreadDatas[i]->threadCommandBuffers.size(); ++j)
			{
				delete m_ThreadDatas[i]->threadCommandBuffers[j];
			}

			vkDestroyCommandPool(m_VulkanDevice->GetInstanceHandle(), m_ThreadDatas[i]->commandPool, VULKAN_CPU_ALLOCATOR);
			delete m_ThreadDatas[i];
		}
		m_ThreadDatas.clear();

		for (int32 i = 0; i < m_Threads.size(); ++i) {
			delete m_Threads[i];
		}
		m_Threads.clear();
	}

	void SetupCommandBuffers(int32 backBufferIndex)
	{
		float w  = m_FrameWidth;
		float h  = m_FrameHeight;
		float tx = 0;
		float ty = 0;

		VkViewport viewport = {};
		viewport.x        = tx;
		viewport.y        = m_FrameHeight - ty;
		viewport.width    = w;
		viewport.height   = -h;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = w;
		scissor.extent.height = h;
		scissor.offset.x = tx;
		scissor.offset.y = ty;

		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferInheritanceInfo cmdBufferInheritanceInfo;
		ZeroVulkanStruct(cmdBufferInheritanceInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO);
		cmdBufferInheritanceInfo.renderPass  = m_RenderPass;
		cmdBufferInheritanceInfo.framebuffer = m_FrameBuffers[backBufferIndex];

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

		vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		RenderUI(cmdBufferInheritanceInfo, backBufferIndex);

		for (int32 i = 0; i < m_ThreadDatas.size(); ++i) {
			vkCmdExecuteCommands(commandBuffer, 1, &(m_ThreadDatas[i]->threadCommandBuffers[backBufferIndex]->cmdBuffer));
		}
		vkCmdExecuteCommands(commandBuffer, 1, &(m_UICommandBuffers[backBufferIndex]->cmdBuffer));

		vkCmdEndRenderPass(commandBuffer);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void RenderUI(VkCommandBufferInheritanceInfo inheritanceInfo, int32 backBufferIndex)
	{
		VkCommandBuffer commandBuffer = m_UICommandBuffers[backBufferIndex]->cmdBuffer;

		VkCommandBufferBeginInfo cmdBufferBeginInfo;
		ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		cmdBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;

		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo));

		float w  = m_FrameWidth;
		float h  = m_FrameHeight;
		float tx = 0;
		float ty = 0;

		VkViewport viewport = {};
		viewport.x        = tx;
		viewport.y        = m_FrameHeight - ty;
		viewport.width    = w;
		viewport.height   = -h;    // flip y axis
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = w;
		scissor.extent.height = h;
		scissor.offset.x = tx;
		scissor.offset.y = ty;

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		// ui pass
		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		vk_demo::DVKBoundingBox bounds = m_RoleModel->rootNode->GetBounds();
		Vector3 boundSize   = bounds.max - bounds.min;
		Vector3 boundCenter = bounds.min + boundSize * 0.5f;
		boundCenter.z -= boundSize.Size() * 1.5f;

		m_ViewCamera.SetPosition(boundCenter);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 1.0f, 1500.0f);

		m_UICommandBuffers.resize(GetVulkanRHI()->GetSwapChain()->GetBackBufferCount());
		for (int32 i = 0; i < m_UICommandBuffers.size(); ++i) {
			m_UICommandBuffers[i] = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		}
	}

	void InitThreads()
	{
		int32 numThreads = std::thread::hardware_concurrency();
		if (numThreads == 0) {
			numThreads = 8;
		}

		if (numThreads > 8) {
			numThreads = 8;
		}

		vk_demo::DVKPrimitive* primitive = m_RoleModel->meshes[0]->primitives[0];

		int32 threadNum = numThreads * 35;
		int32 perThread = primitive->vertexCount / threadNum;
		int32 remainNum = primitive->vertexCount - perThread * threadNum;
		int32 dataIndex = 0;

		m_Particles.resize(threadNum);
		for (int32 i = 0; i < threadNum; ++i)
		{
			int32 count = remainNum > 0 ? perThread + 1 : perThread;
			remainNum -= 1;

			m_Particles[i] = new ParticleModel(m_ParticleModel, m_ParticleMaterial, m_RoleModel, dataIndex, count);
			
			dataIndex += count;
		}

		// thread task
		m_MainFrameID      = 0;
		m_ThreadRunning    = true;

		perThread = m_Particles.size() / numThreads;
		remainNum = m_Particles.size() - perThread * numThreads;
		dataIndex = 0;

		m_ThreadDatas.resize(numThreads);
		m_Threads.resize(numThreads);

		for (int32 i = 0; i < numThreads; ++i)
		{
			// prepare thread data
			m_ThreadDatas[i] = new ThreadData();

			// thread particles
			int32 count = remainNum > 0 ? perThread + 1 : perThread;
			remainNum -= 1;

			for (int32 index = dataIndex; index < dataIndex + count; ++index) {
				m_ThreadDatas[i]->particles.push_back(m_Particles[index]);
			}

			dataIndex += count;

			// command pool per thread
			VkCommandPoolCreateInfo cmdPoolInfo;
			ZeroVulkanStruct(cmdPoolInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
			cmdPoolInfo.queueFamilyIndex = GetVulkanRHI()->GetDevice()->GetPresentQueue()->GetFamilyIndex();
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VERIFYVULKANRESULT(vkCreateCommandPool(m_VulkanDevice->GetInstanceHandle(), &cmdPoolInfo, VULKAN_CPU_ALLOCATOR, &(m_ThreadDatas[i]->commandPool)));

			// command buffers per frame
			m_ThreadDatas[i]->threadCommandBuffers.resize(GetVulkanRHI()->GetSwapChain()->GetBackBufferCount());
			for (int32 index = 0; index < m_ThreadDatas[i]->threadCommandBuffers.size(); ++index) {
				m_ThreadDatas[i]->threadCommandBuffers[index] = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_ThreadDatas[i]->commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
			}

			// start thread
			m_ThreadDatas[i]->index = i;
			m_Threads[i] = new MyThread([=] {
				ThreadRendering(m_ThreadDatas[i]);
			});
		}
	}

	void ThreadRendering(void* param)
	{
		ThreadData* threadData = (ThreadData*)param;
		threadData->frameID = 0;

		while (true)
		{
			{
				std::unique_lock<std::mutex> guardLock(m_FrameStartLock);
				if (threadData->frameID == m_MainFrameID) {
					m_FrameStartCV.wait(guardLock);
				}
			}

			threadData->frameID = m_MainFrameID;

			if (!m_ThreadRunning) {
				break;
			}

			// update particles
			for (int32 i = 0; i < threadData->particles.size(); ++i) {
				threadData->particles[i]->Update(m_BonesData, m_ViewCamera, m_FrameTime, m_FrameDelta);
			}

			// record commands
			VkCommandBuffer commandBuffer = threadData->threadCommandBuffers[m_bufferIndex]->cmdBuffer;

			VkCommandBufferInheritanceInfo cmdBufferInheritanceInfo;
			ZeroVulkanStruct(cmdBufferInheritanceInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO);
			cmdBufferInheritanceInfo.renderPass  = m_RenderPass;
			cmdBufferInheritanceInfo.framebuffer = m_FrameBuffers[m_bufferIndex];

			VkCommandBufferBeginInfo cmdBufferBeginInfo;
			ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
			cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
			cmdBufferBeginInfo.pInheritanceInfo = &cmdBufferInheritanceInfo;

			VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo));

			float w  = m_FrameWidth;
			float h  = m_FrameHeight;
			float tx = 0;
			float ty = 0;

			VkViewport viewport = {};
			viewport.x        = tx;
			viewport.y        = m_FrameHeight - ty;
			viewport.width    = w;
			viewport.height   = -h;    // flip y axis
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor = {};
			scissor.extent.width  = w;
			scissor.extent.height = h;
			scissor.offset.x = tx;
			scissor.offset.y = ty;

			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			for (int32 i = 0; i < threadData->particles.size(); ++i) {
				threadData->particles[i]->Draw(commandBuffer, m_ViewCamera);
			}

			VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));

			// notify thread done
			{
				std::lock_guard<std::mutex> lockGuard(m_ThreadDoneLock);
				m_ThreadDoneCount  += 1;
				m_ThreadDoneCV.notify_one();
			}
		}

		{
			std::lock_guard<std::mutex> lockGuard(writeMutex);
			MLOG("Thread exist -> index = %d", threadData->index);
		}
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

	typedef std::vector<vk_demo::DVKCommandBuffer*> CommandBufferArray;

	bool 						m_Ready = false;

	vk_demo::DVKModel*			m_Quad = nullptr;

	vk_demo::DVKModel*			m_RoleModel = nullptr;
	vk_demo::DVKModel*			m_ParticleModel = nullptr;
	vk_demo::DVKShader*			m_ParticleShader = nullptr;
	vk_demo::DVKTexture*		m_ParticleTexture = nullptr;
	vk_demo::DVKMaterial*		m_ParticleMaterial = nullptr;

	CommandBufferArray			m_UICommandBuffers;
	
	vk_demo::DVKCamera		    m_ViewCamera;

	std::mutex					m_FrameStartLock;
	std::condition_variable		m_FrameStartCV;

	std::mutex					m_ThreadDoneLock;
	std::condition_variable		m_ThreadDoneCV;
	int32						m_ThreadDoneCount;

	ModelViewProjectionBlock	m_MVPParam;
	std::vector<Matrix4x4>		m_BonesData;

	std::vector<ParticleModel*> m_Particles;
	std::vector<ThreadData*>	m_ThreadDatas;
	std::vector<MyThread*>		m_Threads;
	bool						m_ThreadRunning;
	int32						m_MainFrameID;

	float						m_FrameTime;
	float						m_FrameDelta;
	int32						m_bufferIndex;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<ThreadedRenderingDemo>(1400, 900, "ThreadedRenderingDemo", cmdLine);
}
