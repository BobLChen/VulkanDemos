#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>

class PickDemo : public DemoBase
{
public:
	PickDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{

	}

	virtual ~PickDemo()
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

	struct SimpleLine
	{
		std::vector<float> vertices;
		Vector3 lastVert;
		int32 length;
		int32 index;

		SimpleLine()
		{
			
		}

		void Resize(int32 size)
		{
			length = size;
			index  = 0;
			vertices.resize(size);
		}

		void Clear()
		{
			index = 0;
			memset(vertices.data(), 0, vertices.size() * sizeof(float));
		}

		void MoveTo(float x, float y, float z)
		{
			lastVert.x = x;
			lastVert.y = y;
			lastVert.z = z;
		}

		void LineTo(float x, float y, float z)
		{
			if (index + 6 >= length) {
				return;
			}

			vertices[index++] = lastVert.x;
			vertices[index++] = lastVert.y;
			vertices[index++] = lastVert.z;

			vertices[index++] = x;
			vertices[index++] = y;
			vertices[index++] = z;

			MoveTo(x, y, z);
		}
	};

	struct ModelViewProjectionBlock
	{
		Matrix4x4 model;
		Matrix4x4 view;
		Matrix4x4 proj;
	};

	void Draw(float time, float delta)
	{
		int32 bufferIndex = DemoBase::AcquireBackbufferIndex();

		UpdateFPS(time, delta);
		bool hovered = UpdateUI(time, delta);
		UpdateLine(time, delta);

		if (!hovered) {
			m_ViewCamera.Update(time, delta);
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
			ImGui::Begin("PickDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	bool IntersectTriangle(const Vector3& orig, const Vector3& dir, Vector3& v0, Vector3& v1, Vector3& v2, float* t, float* u, float* v)
	{
		Vector3 edge1 = v1 - v0;
		Vector3 edge2 = v2 - v0;

		Vector3 pvec = Vector3::CrossProduct(dir, edge2);

		float det = Vector3::DotProduct(edge1, pvec);

		Vector3 tvec;
		if (det > 0) 
		{
			tvec = orig - v0;
		}
		else
		{
			tvec = v0 - orig;
			det = -det;
		}

		if (det < 0.0001f) {
			return false;
		}
		
		*u = Vector3::DotProduct(tvec, pvec);
		if (*u < 0.0f || *u > det) {
			return false;
		}
		
		Vector3 qvec = Vector3::CrossProduct(tvec, edge1);

		*v = Vector3::DotProduct(dir, qvec);
		if (*v < 0.0f || *u + *v > det) {
			return false;
		}
		
		*t = Vector3::DotProduct(edge2, qvec);

		float fInvDet = 1.0f / det;
		*t *= fInvDet;
		*u *= fInvDet;
		*v *= fInvDet;

		return true;
	}

	void UpdateLine(float time, float delta)
	{
		Matrix4x4 invProj = m_ViewCamera.GetProjection();
		invProj.SetInverse();
		Matrix4x4 invView = m_ViewCamera.GetView();
		invView.SetInverse();
		Vector2 mousePos  = InputManager::GetMousePosition();

		// calc clip space position
		Vector3 clipPos;
		clipPos.x = (mousePos.x / GetWidth() * 2.0f - 1.0f);  //  2D:[0, width]  Clip:[-1,  1]
		clipPos.y = -(mousePos.y / GetHeight() * 2.0f - 1.0f); // 2D:[0, height] Clip:[ 1, -1]
		clipPos.z = 1.0f;

		// clip space to view space
		Vector3 ray = invProj.TransformPosition(clipPos);
		ray.x = ray.x * ray.z;
		ray.y = ray.y * ray.z;

		// view space to world space
		ray = invView.DeltaTransformVector(ray);
		ray = ray.GetSafeNormal();

		// camera position
		Vector3 pos = m_ViewCamera.GetTransform().GetOrigin();

		// collision info
		Vector3 v0;
		Vector3 v1;
		Vector3 v2;
		float dist = MAX_flt;
		float t = 0;
		float u = 0;
		float v = 0;

		bool found = false;
		Vector3 triV0;
		Vector3 triV1;
		Vector3 triV2;

		// collision test
		for (int32 meshID = 0; meshID < m_Model->meshes.size(); ++meshID)
		{
			auto mesh = m_Model->meshes[meshID];
			for (int32 primitiveID = 0; primitiveID < mesh->primitives.size(); ++primitiveID)
			{
				auto pritimive = mesh->primitives[primitiveID];
				int32 stride   = pritimive->vertices.size() / pritimive->vertexCount;

				// test per triangle
				for (int32 idx = 0; idx < pritimive->indices.size(); idx += 3)
				{
					int32 index0 = pritimive->indices[idx + 0] * stride;
					int32 index1 = pritimive->indices[idx + 1] * stride;
					int32 index2 = pritimive->indices[idx + 2] * stride;

					v0.Set(pritimive->vertices[index0 + 0], pritimive->vertices[index0 + 1], pritimive->vertices[index0 + 2]);
					v1.Set(pritimive->vertices[index1 + 0], pritimive->vertices[index1 + 1], pritimive->vertices[index1 + 2]);
					v2.Set(pritimive->vertices[index2 + 0], pritimive->vertices[index2 + 1], pritimive->vertices[index2 + 2]);

					if (IntersectTriangle(pos, ray, v0, v1, v2, &t, &u, &v))
					{
						if (t <= dist)
						{
							dist  = t;
							found = true;
							triV0 = v0;
							triV1 = v1;
							triV2 = v2;
						}
					}
				}
			}
		}

		m_SimpleLine.Clear();

		if (found)
		{
			//Vector3 end = pos + ray * dist;
			//// line
			//m_SimpleLine.MoveTo(pos.x, pos.y, pos.z);
			//m_SimpleLine.LineTo(end.x, end.y, end.z);
			// triangle
			m_SimpleLine.MoveTo(triV0.x, triV0.y, triV0.z);
			m_SimpleLine.LineTo(triV1.x, triV1.y, triV1.z);
			m_SimpleLine.LineTo(triV2.x, triV2.y, triV2.z);
			m_SimpleLine.LineTo(triV0.x, triV0.y, triV0.z);
		}

		m_ModelLine->CopyFrom(m_SimpleLine.vertices.data(), sizeof(float) * m_SimpleLine.vertices.size());
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		m_Model = vk_demo::DVKModel::LoadFromFile(
			"assets/models/simplify_BOTI_Dreamsong_Bridge1.fbx",
			m_VulkanDevice,
			cmdBuffer,
			{ 
				VertexAttribute::VA_Position, 
				VertexAttribute::VA_Normal
			}
		);

		m_Shader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/51_Pick/Solid.vert.spv",
			"assets/shaders/51_Pick/Solid.frag.spv"
		);

		m_Material = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader
		);
		m_Material->PreparePipeline();

		m_SimpleLine.Resize(6 * 128);
		m_ModelLine = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			m_SimpleLine.vertices.size() * sizeof(float),
			m_SimpleLine.vertices.data()
		);
		m_ModelLine->Map();

		m_ShaderLine = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/51_Pick/Line.vert.spv",
			"assets/shaders/51_Pick/Line.frag.spv"
		);

		m_MaterialLine = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_ShaderLine
		);
		m_MaterialLine->pipelineInfo.inputAssemblyState.topology       = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		m_MaterialLine->pipelineInfo.rasterizationState.cullMode       = VK_CULL_MODE_NONE;
		m_MaterialLine->pipelineInfo.rasterizationState.lineWidth      = 1.0f;
		m_MaterialLine->pipelineInfo.rasterizationState.polygonMode    = VK_POLYGON_MODE_LINE;
		m_MaterialLine->pipelineInfo.depthStencilState.depthTestEnable = VK_FALSE;
		m_MaterialLine->PreparePipeline();

		delete cmdBuffer;
	}

	void DestroyAssets()
	{
		delete m_Model;
		delete m_Material;
		delete m_Shader;

		delete m_ModelLine;
		delete m_MaterialLine;
		delete m_ShaderLine;
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

		// render model
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material->GetPipeline());
		m_Material->BeginFrame();
		for (int32 i = 0; i < m_Model->meshes.size(); ++i)
		{
			m_MVPParam.model = m_Model->meshes[i]->linkNode->GetGlobalMatrix();
			m_MVPParam.view  = m_ViewCamera.GetView();
			m_MVPParam.proj  = m_ViewCamera.GetProjection();

			m_Material->BeginObject();
			m_Material->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
			m_Material->EndObject();

			m_Material->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, i);
			m_Model->meshes[i]->BindDrawCmd(commandBuffer);
		}
		m_Material->EndFrame();

		// render line
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_MaterialLine->GetPipeline());
		m_MaterialLine->BeginFrame();

		m_MVPParam.model.SetIdentity();
		m_MVPParam.view  = m_ViewCamera.GetView();
		m_MVPParam.proj  = m_ViewCamera.GetProjection();

		m_MaterialLine->BeginObject();
		m_MaterialLine->SetLocalUniform("uboMVP",      &m_MVPParam,         sizeof(ModelViewProjectionBlock));
		m_MaterialLine->EndObject();
		m_MaterialLine->BindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 0);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &(m_ModelLine->buffer), offsets);
		vkCmdDraw(commandBuffer, m_ModelLine->size / sizeof(float), 1, 0, 0);

		m_MaterialLine->EndFrame();
		
		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);
		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

	void InitParmas()
	{
		m_ViewCamera.SetPosition(0, 500, -700.0f);
		m_ViewCamera.LookAt(0, 250, 0);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 1.0f, 1500.0f);
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

	SimpleLine					m_SimpleLine;
	vk_demo::DVKBuffer*			m_ModelLine = nullptr;
	vk_demo::DVKMaterial*		m_MaterialLine = nullptr;
	vk_demo::DVKShader*			m_ShaderLine = nullptr;

	vk_demo::DVKModel*			m_Model = nullptr;
	vk_demo::DVKMaterial*		m_Material = nullptr;
	vk_demo::DVKShader*			m_Shader = nullptr;

	vk_demo::DVKCamera		    m_ViewCamera;

	ModelViewProjectionBlock	m_MVPParam;

	ImageGUIContext*			m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<PickDemo>(1400, 900, "PickDemo", cmdLine);
}
