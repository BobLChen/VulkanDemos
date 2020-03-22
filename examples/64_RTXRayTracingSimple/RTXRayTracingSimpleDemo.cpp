#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include <vector>

struct Node;
struct Material;
struct Mesh;

// Geometry instance, with the layout expected by VK_NV_ray_tracing
struct VkGeometryInstance
{
	// Transform matrix, containing only the top 3 rows
	float transform[12];
	// Instance index
	uint32_t instanceId : 24;
	// Visibility mask
	uint32_t mask : 8;
	// Index of the hit group which will be invoked when a ray hits the instance
	uint32_t instanceOffset : 24;
	// Instance flags, such as culling
	uint32_t flags : 8;
	// Opaque handle of the bottom-level acceleration structure
	uint64_t accelerationStructureHandle;
};

struct CameraParamBlock
{
	Vector4 lens;
	Vector4 pos;
	IntVector4 samplesAndSeed;
	Matrix4x4 invProj;
	Matrix4x4 invView;
};

struct AccelerationStructureInstance
{
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkAccelerationStructureNV accelerationStructure = VK_NULL_HANDLE;
	uint64 handle = 0;
};

struct Material
{
	Vector4 albedo = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // base color
	Vector4 params = Vector4(1.0f, 0.0f, 1.0f, 0.0f); // roughness, metallic, occlusion, padding
	IntVector4 textureIDs = IntVector4(-1, -1, -1, -1); // albedo, roughness, metallic, padding
};

struct Mesh
{
	uint32 vertexCount = 0;
	uint32 vertexStride = 0;
	vk_demo::DVKBuffer* vertexBuffer = nullptr;

	uint32 indexCount = 0;
	vk_demo::DVKBuffer* indexBuffer = nullptr;

	int32 material = -1;

	~Mesh()
	{
		if (vertexBuffer) {
			delete vertexBuffer;
			vertexBuffer = nullptr;
		}

		if (indexBuffer) {
			delete indexBuffer;
			indexBuffer = nullptr;
		}
	}
};

struct Node
{
	std::string name;

	Matrix4x4 transform;

	int32 mesh = -1;

	Node* parent = nullptr;
	std::vector<Node*> children;

	Matrix4x4 GetWorldTransform()
	{
		Matrix4x4 worldTransform = transform;
		if (parent) {
			worldTransform.Append(parent->GetWorldTransform());
		}
		return worldTransform;
	}
};

struct ObjectInstance
{
	IntVector4 params = IntVector4(-1, -1, -1, -1); // material、mesh、padding、padding
};

struct Scene
{
	Node* rootNode = nullptr;

	std::vector<Node*> nodes;
	std::vector<vk_demo::DVKTexture*> textures;
	std::vector<Material> materials;
	std::vector<Mesh*> meshes;
	std::vector<Node*> entities;

	void Destroy()
	{
		for (int32 i = 0; i < nodes.size(); ++i) {
			delete nodes[i];
		}
		nodes.clear();

		for (int32 i = 0; i < textures.size(); ++i) {
			delete textures[i];
		}
		textures.clear();

		for (int32 i = 0; i < meshes.size(); ++i) {
			delete meshes[i];
		}
		meshes.clear();
	}
};

class RTXRayTracingSimpleDemo : public DemoBase
{
public:
	RTXRayTracingSimpleDemo(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
		deviceExtensions.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
		instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		ZeroVulkanStruct(m_IndexingFeatures, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES);
		m_IndexingFeatures.pNext = nullptr;
		m_IndexingFeatures.runtimeDescriptorArray = true;

		ZeroVulkanStruct(m_EnabledFeatures2, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);
		m_EnabledFeatures2.pNext = &m_IndexingFeatures;

		physicalDeviceFeatures = &m_EnabledFeatures2;
	}

	virtual ~RTXRayTracingSimpleDemo()
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
		LoadExtensions();
		LoadAssets();
		PrepareUniformBuffers();
		PrepareAS();
		PrepareRayTracingPipeline();
		PrepareShaderBindingTab();
		PrepareDescriptorSets();

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

		UpdateFPS(time, delta);

		bool hovered = UpdateUI(time, delta);
		if (!hovered) {
			m_ViewCamera.Update(time, delta);
		}

		UpdateUniformBuffer();
		SetupGfxCommand(bufferIndex);

		DemoBase::Present(bufferIndex);
	}

	bool UpdateUI(float time, float delta)
	{
		m_GUI->StartFrame();

		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
			ImGui::Begin("RTXRaytracing", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

			ImGui::SliderInt("NumberOfSamples", &m_CameraParam.samplesAndSeed.x, 1, 128);
			ImGui::SliderInt("NumberOfBounces", &m_CameraParam.samplesAndSeed.y, 1, 128);
			ImGui::SliderInt("Seed", &m_CameraParam.samplesAndSeed.z, 1, 65535);

			ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / m_LastFPS, m_LastFPS);
			ImGui::End();
		}

		bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

		m_GUI->EndFrame();
		m_GUI->Update();

		return hovered;
	}

	void UpdateUniformBuffer()
	{
		float yMaxFar = m_ViewCamera.GetFar() * MMath::Tan(m_ViewCamera.GetFov() / 2);
		float xMaxFar = yMaxFar * (float)GetWidth() / (float)GetHeight();

		m_CameraParam.lens.x = xMaxFar;
		m_CameraParam.lens.y = yMaxFar;
		m_CameraParam.lens.z = m_ViewCamera.GetNear();
		m_CameraParam.lens.w = m_ViewCamera.GetFar();

		m_CameraParam.pos = m_ViewCamera.GetTransform().GetOrigin();

		m_CameraParam.samplesAndSeed.w += 1;
		
		m_CameraParam.invProj = m_ViewCamera.GetProjection();
		m_CameraParam.invProj.SetInverse();
		m_CameraParam.invView = m_ViewCamera.GetView();
		m_CameraParam.invView.SetInverse();

		memcpy(m_UniformBuffer->mapped, &m_CameraParam, sizeof(CameraParamBlock));
	}

	void PrepareDescriptorSets()
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		m_DescriptorSets.resize(m_DescriptorSetLayouts.size());

		// sets
		std::vector<VkDescriptorPoolSize> poolSizes(6);
		// set=0,accelerationStructureNV
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
		poolSizes[0].descriptorCount = 1;
		// set=0,image
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes[1].descriptorCount = 1;
		// set=0,CameraProperties
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[2].descriptorCount = 1;
		// set=1,Vertices + Indices
		poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[3].descriptorCount = 2 * m_Scene.meshes.size();
		// set=1,Materials + ObjectInstances
		poolSizes[4].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[4].descriptorCount = 2;
		// set=1,textures
		poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[5].descriptorCount = 1 * m_Scene.meshes.size();;

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		ZeroVulkanStruct(descriptorPoolCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
		descriptorPoolCreateInfo.maxSets = m_DescriptorSets.size();
		VERIFYVULKANRESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		ZeroVulkanStruct(descriptorSetAllocateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
		descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = m_DescriptorSetLayouts.data();
		descriptorSetAllocateInfo.descriptorSetCount = m_DescriptorSetLayouts.size();
		VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, m_DescriptorSets.data()));

		std::vector<VkWriteDescriptorSet> writeDescriptorSets;

		// set0
		// topLevelAS
		VkWriteDescriptorSetAccelerationStructureNV descriptorASNV;
		ZeroVulkanStruct(descriptorASNV, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV);
		descriptorASNV.accelerationStructureCount = 1;
		descriptorASNV.pAccelerationStructures = &m_TopLevelAS.accelerationStructure;
		VkWriteDescriptorSet asWriteDescriptorSet;
		ZeroVulkanStruct(asWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		asWriteDescriptorSet.pNext = &descriptorASNV;
		asWriteDescriptorSet.dstSet = m_DescriptorSets[0];
		asWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
		asWriteDescriptorSet.dstBinding = 0;
		asWriteDescriptorSet.descriptorCount = 1;

		// image
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageView = m_StorageImage->imageView;
		imageInfo.imageLayout = m_StorageImage->imageLayout;
		VkWriteDescriptorSet imageWriteDescriptorSet;
		ZeroVulkanStruct(imageWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		imageWriteDescriptorSet.pImageInfo = &imageInfo;
		imageWriteDescriptorSet.dstSet = m_DescriptorSets[0];
		imageWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageWriteDescriptorSet.dstBinding = 1;
		imageWriteDescriptorSet.descriptorCount = 1;

		// CameraProperties
		VkWriteDescriptorSet uboWriteDescriptorSet;
		ZeroVulkanStruct(uboWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		uboWriteDescriptorSet.pBufferInfo = &m_UniformBuffer->descriptor;
		uboWriteDescriptorSet.dstSet = m_DescriptorSets[0];
		uboWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboWriteDescriptorSet.dstBinding = 2;
		uboWriteDescriptorSet.descriptorCount = 1;

		writeDescriptorSets.push_back(asWriteDescriptorSet);
		writeDescriptorSets.push_back(imageWriteDescriptorSet);
		writeDescriptorSets.push_back(uboWriteDescriptorSet);

		// set 1
		// vertices
		std::vector<VkDescriptorBufferInfo> vertexBufferInfos(m_Scene.meshes.size());
		for (int32 i = 0; i < m_Scene.meshes.size(); ++i)
		{
			vertexBufferInfos[i].buffer = m_Scene.meshes[i]->vertexBuffer->buffer;
			vertexBufferInfos[i].offset = 0;
			vertexBufferInfos[i].range = VK_WHOLE_SIZE;
		}
		VkWriteDescriptorSet vertexWriteDescriptorSet;
		ZeroVulkanStruct(vertexWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		vertexWriteDescriptorSet.pBufferInfo = vertexBufferInfos.data();
		vertexWriteDescriptorSet.dstSet = m_DescriptorSets[1];
		vertexWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		vertexWriteDescriptorSet.dstBinding = 0;
		vertexWriteDescriptorSet.descriptorCount = vertexBufferInfos.size();

		// indices
		std::vector<VkDescriptorBufferInfo> indexBufferInfos(m_Scene.meshes.size());
		for (int32 i = 0; i < m_Scene.meshes.size(); ++i)
		{
			indexBufferInfos[i].buffer = m_Scene.meshes[i]->indexBuffer->buffer;
			indexBufferInfos[i].offset = 0;
			indexBufferInfos[i].range = VK_WHOLE_SIZE;
		}
		VkWriteDescriptorSet indexWriteDescriptorSet;
		ZeroVulkanStruct(indexWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		indexWriteDescriptorSet.pBufferInfo = indexBufferInfos.data();
		indexWriteDescriptorSet.dstSet = m_DescriptorSets[1];
		indexWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		indexWriteDescriptorSet.dstBinding = 1;
		indexWriteDescriptorSet.descriptorCount = indexBufferInfos.size();

		// materials
		VkDescriptorBufferInfo materialsBufferInfo;
		materialsBufferInfo.buffer = m_MaterialsBuffer->buffer;
		materialsBufferInfo.offset = 0;
		materialsBufferInfo.range = VK_WHOLE_SIZE;
		VkWriteDescriptorSet materiaslWriteDescriptorSet;
		ZeroVulkanStruct(materiaslWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		materiaslWriteDescriptorSet.pBufferInfo = &materialsBufferInfo;
		materiaslWriteDescriptorSet.dstSet = m_DescriptorSets[1];
		materiaslWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		materiaslWriteDescriptorSet.dstBinding = 2;
		materiaslWriteDescriptorSet.descriptorCount = 1;

		// objects
		VkDescriptorBufferInfo objectsBufferInfo;
		objectsBufferInfo.buffer = m_ObjectsBuffer->buffer;
		objectsBufferInfo.offset = 0;
		objectsBufferInfo.range = VK_WHOLE_SIZE;
		VkWriteDescriptorSet objectsWriteDescriptorSet;
		ZeroVulkanStruct(objectsWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		objectsWriteDescriptorSet.pBufferInfo = &objectsBufferInfo;
		objectsWriteDescriptorSet.dstSet = m_DescriptorSets[1];
		objectsWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		objectsWriteDescriptorSet.dstBinding = 3;
		objectsWriteDescriptorSet.descriptorCount = 1;

		// textures
		std::vector<VkDescriptorImageInfo> textureImageInfos(m_Scene.textures.size());
		for (int32 i = 0; i < m_Scene.textures.size(); ++i)
		{
			textureImageInfos[i] = m_Scene.textures[i]->descriptorInfo;
		}
		VkWriteDescriptorSet textureWriteDescriptorSet;
		ZeroVulkanStruct(textureWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		textureWriteDescriptorSet.pImageInfo = textureImageInfos.data();
		textureWriteDescriptorSet.dstSet = m_DescriptorSets[1];
		textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureWriteDescriptorSet.dstBinding = 4;
		textureWriteDescriptorSet.descriptorCount = m_Scene.textures.size();

		writeDescriptorSets.push_back(vertexWriteDescriptorSet);
		writeDescriptorSets.push_back(indexWriteDescriptorSet);
		writeDescriptorSets.push_back(materiaslWriteDescriptorSet);
		writeDescriptorSets.push_back(objectsWriteDescriptorSet);
		writeDescriptorSets.push_back(textureWriteDescriptorSet);

		vkUpdateDescriptorSets(device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
	}

	void PrepareShaderBindingTab()
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		const uint32 shaderGroupHandleSize = m_RayTracingPropertiesNV.shaderGroupHandleSize * 3;
		std::vector<uint8> shaderGroupHandleData(shaderGroupHandleSize);
		VERIFYVULKANRESULT(vkGetRayTracingShaderGroupHandlesNV(device, m_Pipeline, 0, 3, shaderGroupHandleSize, shaderGroupHandleData.data()));

		m_ShaderBindingTable = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice,
			VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			shaderGroupHandleSize,
			shaderGroupHandleData.data()
		);
	}

	void PrepareRayTracingPipeline()
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		m_DescriptorSetLayouts.resize(2);

		// set0
		{
			// topLevelAS
			VkDescriptorSetLayoutBinding asLayoutBinding = {};
			asLayoutBinding.binding = 0;
			asLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
			asLayoutBinding.descriptorCount = 1;
			asLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
			// image
			VkDescriptorSetLayoutBinding imageLayoutBinding = {};
			imageLayoutBinding.binding = 1;
			imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			imageLayoutBinding.descriptorCount = 1;
			imageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;
			// CameraProperties
			VkDescriptorSetLayoutBinding uniformBufferBinding = {};
			uniformBufferBinding.binding = 2;
			uniformBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uniformBufferBinding.descriptorCount = 1;
			uniformBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;

			std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
			layoutBindings.push_back(asLayoutBinding);
			layoutBindings.push_back(imageLayoutBinding);
			layoutBindings.push_back(uniformBufferBinding);

			VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
			ZeroVulkanStruct(layoutCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
			layoutCreateInfo.bindingCount = layoutBindings.size();
			layoutCreateInfo.pBindings = layoutBindings.data();
			VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayouts[0]));
		}

		// set1
		{
			std::vector<VkDescriptorSetLayoutBinding> layoutBindings(5);
			// Vertices
			layoutBindings[0].binding = 0;
			layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layoutBindings[0].descriptorCount = m_Scene.meshes.size();
			layoutBindings[0].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
			// Indices
			layoutBindings[1].binding = 1;
			layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layoutBindings[1].descriptorCount = m_Scene.meshes.size();
			layoutBindings[1].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
			// Materials
			layoutBindings[2].binding = 2;
			layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layoutBindings[2].descriptorCount = 1;
			layoutBindings[2].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
			// ObjectInstances
			layoutBindings[3].binding = 3;
			layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layoutBindings[3].descriptorCount = 1;
			layoutBindings[3].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
			// textures
			layoutBindings[4].binding = 4;
			layoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layoutBindings[4].descriptorCount = m_Scene.textures.size();
			layoutBindings[4].stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;

			VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
			ZeroVulkanStruct(layoutCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
			layoutCreateInfo.bindingCount = layoutBindings.size();
			layoutCreateInfo.pBindings = layoutBindings.data();
			VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayouts[1]));
		}

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		ZeroVulkanStruct(pipelineLayoutCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipelineLayoutCreateInfo.setLayoutCount = m_DescriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = m_DescriptorSetLayouts.data();
		VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));

		auto rayGenShaderModule = vk_demo::DVKShaderModule::Create(m_VulkanDevice, "assets/shaders/64_RTXRayTracingSimple/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_NV);
		auto rayMisShaderModule = vk_demo::DVKShaderModule::Create(m_VulkanDevice, "assets/shaders/64_RTXRayTracingSimple/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_NV);
		auto rayHitShaderModule = vk_demo::DVKShaderModule::Create(m_VulkanDevice, "assets/shaders/64_RTXRayTracingSimple/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages(3);
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].flags = 0;
		shaderStages[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_NV;
		shaderStages[0].module = rayGenShaderModule->handle;
		shaderStages[0].pName = "main";
		shaderStages[0].pSpecializationInfo = nullptr;

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].flags = 0;
		shaderStages[1].stage = VK_SHADER_STAGE_MISS_BIT_NV;
		shaderStages[1].module = rayMisShaderModule->handle;
		shaderStages[1].pName = "main";
		shaderStages[1].pSpecializationInfo = nullptr;

		shaderStages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[2].pNext = nullptr;
		shaderStages[2].flags = 0;
		shaderStages[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
		shaderStages[2].module = rayHitShaderModule->handle;
		shaderStages[2].pName = "main";
		shaderStages[2].pSpecializationInfo = nullptr;

		std::vector<VkRayTracingShaderGroupCreateInfoNV> shaderGroups(3);
		shaderGroups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
		shaderGroups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
		shaderGroups[0].pNext = nullptr;
		shaderGroups[0].generalShader = 0;
		shaderGroups[0].closestHitShader = VK_SHADER_UNUSED_NV;
		shaderGroups[0].anyHitShader = VK_SHADER_UNUSED_NV;
		shaderGroups[0].intersectionShader = VK_SHADER_UNUSED_NV;

		shaderGroups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
		shaderGroups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
		shaderGroups[1].pNext = nullptr;
		shaderGroups[1].generalShader = 1;
		shaderGroups[1].closestHitShader = VK_SHADER_UNUSED_NV;
		shaderGroups[1].anyHitShader = VK_SHADER_UNUSED_NV;
		shaderGroups[1].intersectionShader = VK_SHADER_UNUSED_NV;

		shaderGroups[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
		shaderGroups[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
		shaderGroups[2].pNext = nullptr;
		shaderGroups[2].generalShader = VK_SHADER_UNUSED_NV;
		shaderGroups[2].closestHitShader = 2;
		shaderGroups[2].anyHitShader = VK_SHADER_UNUSED_NV;
		shaderGroups[2].intersectionShader = VK_SHADER_UNUSED_NV;

		VkRayTracingPipelineCreateInfoNV pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV);
		pipelineCreateInfo.stageCount = shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.groupCount = shaderGroups.size();
		pipelineCreateInfo.pGroups = shaderGroups.data();
		pipelineCreateInfo.maxRecursionDepth = 8;
		pipelineCreateInfo.layout = m_PipelineLayout;
		VERIFYVULKANRESULT(vkCreateRayTracingPipelinesNV(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));

		delete rayGenShaderModule;
		delete rayMisShaderModule;
		delete rayHitShaderModule;
	}

	void PrepareUniformBuffers()
	{
		m_CameraParam.samplesAndSeed.x = 16;
		m_CameraParam.samplesAndSeed.y = 16;
		m_CameraParam.samplesAndSeed.z = MMath::RandRange(0, 65535);
		m_CameraParam.samplesAndSeed.w = 0;

		m_UniformBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(CameraParamBlock),
			&m_CameraParam
		);
		m_UniformBuffer->Map();

		m_ViewCamera.SetPosition(Vector3(2.97830200f, 42.5552597f, 53.1424141f));
		m_ViewCamera.SetRotation(Vector3(-145.999985f, -1.49999976f, 180.000000f));
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 0.1f, 1000.0f);
	}

	void LoadTextures(vk_demo::DVKCommandBuffer* cmdBuffer, tinygltf::Model &gltfModel)
	{
		for (int32 i = 0; i < gltfModel.textures.size(); ++i)
		{
			tinygltf::Texture& tex = gltfModel.textures[i];
			tinygltf::Image& image = gltfModel.images[tex.source];
			vk_demo::DVKTexture* texture = vk_demo::DVKTexture::Create2D(image.image.data(), image.width * image.height * 4, VK_FORMAT_R8G8B8A8_UNORM, image.width, image.height, m_VulkanDevice, cmdBuffer);
			m_Scene.textures.push_back(texture);
		}
	}

	void LoadMaterials(vk_demo::DVKCommandBuffer* cmdBuffer, tinygltf::Model &gltfModel)
	{
		for (int32 i = 0; i < gltfModel.materials.size(); ++i)
		{
			tinygltf::Material& gltfMat = gltfModel.materials[i];
			Material material;

			if (gltfMat.values.find("baseColorTexture") != gltfMat.values.end()) {
				material.textureIDs.x = gltfMat.values["baseColorTexture"].TextureIndex();
			}

			if (gltfMat.values.find("roughnessFactor") != gltfMat.values.end()) {
				material.params.x = gltfMat.values["roughnessFactor"].Factor();
			}

			if (gltfMat.values.find("metallicFactor") != gltfMat.values.end()) {
				material.params.y = gltfMat.values["metallicFactor"].Factor();
			}

			if (gltfMat.values.find("baseColorFactor") != gltfMat.values.end()) {
				material.albedo.x = gltfMat.values["baseColorFactor"].ColorFactor().data()[0];
				material.albedo.y = gltfMat.values["baseColorFactor"].ColorFactor().data()[1];
				material.albedo.z = gltfMat.values["baseColorFactor"].ColorFactor().data()[2];
				material.albedo.w = gltfMat.values["baseColorFactor"].ColorFactor().data()[3];
			}

			m_Scene.materials.push_back(material);
		}

		// prepare material buffer
		m_MaterialsBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(Material) * m_Scene.materials.size(),
			m_Scene.materials.data()
		);
	}

	void LoadMeshes(vk_demo::DVKCommandBuffer* cmdBuffer, tinygltf::Model &gltfModel)
	{
		for (int32 i = 0; i < gltfModel.meshes.size(); ++i)
		{
			tinygltf::Mesh& gltfMesh = gltfModel.meshes[i];

			for (int32 j = 0; j < gltfMesh.primitives.size(); ++j)
			{
				tinygltf::Primitive& primitive = gltfMesh.primitives[j];

				std::vector<float> vertices;
				std::vector<uint32> indices;

				// vertices
				uint8* bufferPos = nullptr;
				uint8* bufferNormals = nullptr;
				uint8* bufferUV0 = nullptr;
				uint8* bufferTangents = nullptr;

				tinygltf::Accessor& posAccessor = gltfModel.accessors[primitive.attributes.find("POSITION")->second];
				tinygltf::BufferView& posView = gltfModel.bufferViews[posAccessor.bufferView];
				bufferPos = &(gltfModel.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]);

				tinygltf::Accessor& normAccessor = gltfModel.accessors[primitive.attributes.find("NORMAL")->second];
				tinygltf::BufferView& normView = gltfModel.bufferViews[normAccessor.bufferView];
				bufferNormals = &(gltfModel.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]);

				tinygltf::Accessor& uvAccessor = gltfModel.accessors[primitive.attributes.find("TEXCOORD_0")->second];
				tinygltf::BufferView& uvView = gltfModel.bufferViews[uvAccessor.bufferView];
				bufferUV0 = &(gltfModel.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]);

				tinygltf::Accessor& tangentAccessor = gltfModel.accessors[primitive.attributes.find("TANGENT")->second];
				tinygltf::BufferView& tangentView = gltfModel.bufferViews[tangentAccessor.bufferView];
				bufferTangents = &(gltfModel.buffers[tangentView.buffer].data[tangentAccessor.byteOffset + tangentView.byteOffset]);

				for (int32 v = 0; v < posAccessor.count; ++v)
				{
					// pos
					{
						const float* buf = (const float*)(bufferPos);
						vertices.push_back(buf[v * 3 + 0]);
						vertices.push_back(buf[v * 3 + 1]);
						vertices.push_back(buf[v * 3 + 2]);
					}
					// uv
					{
						const float* buf = (const float*)(bufferUV0);
						vertices.push_back(buf[v * 2 + 0]);
						vertices.push_back(buf[v * 2 + 1]);
					}
					// normal
					{
						const float* buf = (const float*)(bufferNormals);
						vertices.push_back(buf[v * 3 + 0]);
						vertices.push_back(buf[v * 3 + 1]);
						vertices.push_back(buf[v * 3 + 2]);
					}
					// tangent
					{
						const float* buf = (const float*)(bufferTangents);
						vertices.push_back(buf[v * 3 + 0]);
						vertices.push_back(buf[v * 3 + 1]);
						vertices.push_back(buf[v * 3 + 2]);
					}
				}

				// indices
				tinygltf::Accessor& indicesAccessor = gltfModel.accessors[primitive.indices];
				tinygltf::BufferView& indicesBufferView = gltfModel.bufferViews[indicesAccessor.bufferView];
				uint8* bufferIndices = &(gltfModel.buffers[indicesBufferView.buffer].data[indicesAccessor.byteOffset + indicesBufferView.byteOffset]);

				for (int32 v = 0; v < indicesAccessor.count; ++v)
				{
					if (indicesAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)
					{
						const uint32* buf = (const uint32*)(bufferIndices);
						indices.push_back(buf[v]);
					}
					else if (indicesAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
					{
						const uint16* buf = (const uint16*)(bufferIndices);
						indices.push_back(buf[v]);
					}
					else if (indicesAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
					{
						const uint8* buf = (const uint8*)(bufferIndices);
						indices.push_back(buf[v]);
					}
				}

				Mesh* mesh = new Mesh();
				mesh->vertexCount = vertices.size();
				{
					vk_demo::DVKBuffer* vertexStaging = vk_demo::DVKBuffer::CreateBuffer(
						m_VulkanDevice, 
						VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
						vertices.size() * sizeof(float), 
						vertices.data()
					);

					mesh->vertexBuffer = vk_demo::DVKBuffer::CreateBuffer(
						m_VulkanDevice, 
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
						vertices.size() * sizeof(float)
					);

					cmdBuffer->Begin();

					VkBufferCopy copyRegion = {};
					copyRegion.size = vertices.size() * sizeof(float);
					vkCmdCopyBuffer(cmdBuffer->cmdBuffer, vertexStaging->buffer, mesh->vertexBuffer->buffer, 1, &copyRegion);

					cmdBuffer->End();
					cmdBuffer->Submit();

					delete vertexStaging;
				}

				mesh->indexCount = indices.size();
				{
					vk_demo::DVKBuffer* indexStaging = vk_demo::DVKBuffer::CreateBuffer(
						m_VulkanDevice, 
						VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
						indices.size() * sizeof(uint32), 
						indices.data()
					);

					mesh->indexBuffer = vk_demo::DVKBuffer::CreateBuffer(
						m_VulkanDevice, 
						VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
						indices.size() * sizeof(uint32)
					);

					cmdBuffer->Begin();

					VkBufferCopy copyRegion = {};
					copyRegion.size = indices.size() * sizeof(uint32);

					vkCmdCopyBuffer(cmdBuffer->cmdBuffer, indexStaging->buffer, mesh->indexBuffer->buffer, 1, &copyRegion);

					cmdBuffer->End();
					cmdBuffer->Submit();

					delete indexStaging;
				}

				mesh->material = primitive.material;
				m_Scene.meshes.push_back(mesh);
			}
		}
	}

	void LoadNode(Node* parent, tinygltf::Node& gltfNode, tinygltf::Model &gltfModel)
	{
		Node* node = new Node();
		node->name = gltfNode.name;
		node->transform.SetIdentity();

		m_Scene.nodes.push_back(node);

		if (gltfNode.rotation.size() == 4) 
		{
			Quat quat(gltfNode.rotation[0], gltfNode.rotation[1], gltfNode.rotation[2], gltfNode.rotation[3]);
			node->transform.Append(quat.ToMatrix());
		}

		if (gltfNode.scale.size() == 3) 
		{
			node->transform.AppendScale(Vector3(gltfNode.scale[0], gltfNode.scale[1], gltfNode.scale[2]));
		}

		if (gltfNode.translation.size() == 3) 
		{
			node->transform.AppendTranslation(Vector3(gltfNode.translation[0], gltfNode.translation[1], gltfNode.translation[2]));
		}

		node->parent = parent;
		if (parent) 
		{
			parent->children.push_back(node);
		}

		if (gltfNode.mesh > -1)
		{
			node->mesh = gltfNode.mesh;

			m_Scene.entities.push_back(node);
		}

		for (int32 i = 0; i < gltfNode.children.size(); ++i)
		{
			LoadNode(node, gltfModel.nodes[gltfNode.children[i]], gltfModel);
		}
	}

	void LoadNodes(vk_demo::DVKCommandBuffer* cmdBuffer, tinygltf::Model &gltfModel)
	{
		tinygltf::Scene &scene = gltfModel.scenes[0];

		m_Scene.rootNode = new Node();

		for (int32 i = 0; i < scene.nodes.size(); ++i)
		{
			tinygltf::Node& gltfNode = gltfModel.nodes[scene.nodes[i]];
			LoadNode(m_Scene.rootNode, gltfNode, gltfModel);
		}

		// prepare entities buffer
		std::vector<ObjectInstance> objects(m_Scene.entities.size());
		for (int32 i = 0; i < m_Scene.entities.size(); ++i)
		{
			Mesh* mesh = m_Scene.meshes[m_Scene.entities[i]->mesh];
			objects[i].params.x = mesh->material;
			objects[i].params.y = m_Scene.entities[i]->mesh;
		}

		m_ObjectsBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(ObjectInstance) * objects.size(),
			objects.data()
		);
	}

	void LoadGLTFModel(vk_demo::DVKCommandBuffer* cmdBuffer)
	{
		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF gltfContext;
		std::string error;
		std::string warning;

		uint32 dataSize = 0;
		uint8* dataPtr  = nullptr;
		FileManager::ReadFile("assets/models/diorama/diorama.glb", dataPtr, dataSize);

		gltfContext.LoadBinaryFromMemory(&gltfModel, &error, &warning, dataPtr, dataSize);

		LoadTextures(cmdBuffer, gltfModel);
		LoadMaterials(cmdBuffer, gltfModel);
		LoadMeshes(cmdBuffer, gltfModel);
		LoadNodes(cmdBuffer, gltfModel);
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		LoadGLTFModel(cmdBuffer);

		m_Quad = vk_demo::DVKDefaultRes::fullQuad;

		m_StorageImage = vk_demo::DVKTexture::Create2D(
			m_VulkanDevice,
			cmdBuffer,
			m_SwapChain->GetColorFormat(),
			VK_IMAGE_ASPECT_COLOR_BIT,
			m_FrameWidth, m_FrameHeight,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			ImageLayoutBarrier::ComputeGeneralRW
		);

		m_Shader = vk_demo::DVKShader::Create(
			m_VulkanDevice,
			true,
			"assets/shaders/64_RTXRayTracingSimple/result.vert.spv",
			"assets/shaders/64_RTXRayTracingSimple/result.frag.spv"
		);

		m_Material = vk_demo::DVKMaterial::Create(
			m_VulkanDevice,
			m_RenderPass,
			m_PipelineCache,
			m_Shader
		);
		m_Material->PreparePipeline();
		m_Material->SetTexture("diffuseMap", m_StorageImage);

		delete cmdBuffer;
	}

	void LoadExtensions()
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		vkCreateAccelerationStructureNV = reinterpret_cast<PFN_vkCreateAccelerationStructureNV>(vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureNV"));
		vkDestroyAccelerationStructureNV = reinterpret_cast<PFN_vkDestroyAccelerationStructureNV>(vkGetDeviceProcAddr(device, "vkDestroyAccelerationStructureNV"));
		vkBindAccelerationStructureMemoryNV = reinterpret_cast<PFN_vkBindAccelerationStructureMemoryNV>(vkGetDeviceProcAddr(device, "vkBindAccelerationStructureMemoryNV"));
		vkGetAccelerationStructureHandleNV = reinterpret_cast<PFN_vkGetAccelerationStructureHandleNV>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureHandleNV"));
		vkGetAccelerationStructureMemoryRequirementsNV = reinterpret_cast<PFN_vkGetAccelerationStructureMemoryRequirementsNV>(vkGetDeviceProcAddr(device, "vkGetAccelerationStructureMemoryRequirementsNV"));
		vkCmdBuildAccelerationStructureNV = reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructureNV"));
		vkCreateRayTracingPipelinesNV = reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(vkGetDeviceProcAddr(device, "vkCreateRayTracingPipelinesNV"));
		vkGetRayTracingShaderGroupHandlesNV = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesNV>(vkGetDeviceProcAddr(device, "vkGetRayTracingShaderGroupHandlesNV"));
		vkCmdTraceRaysNV = reinterpret_cast<PFN_vkCmdTraceRaysNV>(vkGetDeviceProcAddr(device, "vkCmdTraceRaysNV"));

		ZeroVulkanStruct(m_RayTracingPropertiesNV, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV);

		VkPhysicalDeviceProperties2 deviceProperties2;
		ZeroVulkanStruct(deviceProperties2, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2);
		deviceProperties2.pNext = &m_RayTracingPropertiesNV;
		vkGetPhysicalDeviceProperties2(m_VulkanDevice->GetPhysicalHandle(), &deviceProperties2);
	}

	void PrepareAS()
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

		// bottom level
		CreateBottomLevelAS(cmdBuffer);

		// top level
		CreateTopLevelAS(cmdBuffer);

		delete cmdBuffer;
	}

	void CreateTopLevelAS(vk_demo::DVKCommandBuffer* cmdBuffer)
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		VkAccelerationStructureInfoNV accelerationStructureInfo;
		ZeroVulkanStruct(accelerationStructureInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV);
		accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
		accelerationStructureInfo.instanceCount = m_Scene.entities.size();

		VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo;
		ZeroVulkanStruct(accelerationStructureCreateInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV);
		accelerationStructureCreateInfo.info = accelerationStructureInfo;
		VERIFYVULKANRESULT(vkCreateAccelerationStructureNV(device, &accelerationStructureCreateInfo, VULKAN_CPU_ALLOCATOR, &m_TopLevelAS.accelerationStructure));

		VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo;
		ZeroVulkanStruct(memoryRequirementsInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV);
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
		memoryRequirementsInfo.accelerationStructure = m_TopLevelAS.accelerationStructure;

		VkMemoryRequirements2 memoryRequirements2;
		vkGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo, &memoryRequirements2);

		uint32 memoryTypeIndex = 0;
		m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);

		VkMemoryAllocateInfo memoryAllocateInfo;
		ZeroVulkanStruct(memoryAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memoryAllocateInfo, VULKAN_CPU_ALLOCATOR, &m_TopLevelAS.memory));

		VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo;
		ZeroVulkanStruct(accelerationStructureMemoryInfo, VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV);
		accelerationStructureMemoryInfo.accelerationStructure = m_TopLevelAS.accelerationStructure;
		accelerationStructureMemoryInfo.memory = m_TopLevelAS.memory;
		VERIFYVULKANRESULT(vkBindAccelerationStructureMemoryNV(device, 1, &accelerationStructureMemoryInfo));

		VERIFYVULKANRESULT(vkGetAccelerationStructureHandleNV(device, m_TopLevelAS.accelerationStructure, sizeof(uint64_t), &m_TopLevelAS.handle));

		// scratch size
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
		memoryRequirementsInfo.accelerationStructure = m_TopLevelAS.accelerationStructure;

		VkMemoryRequirements2 topLevelASMemoryRequirements2;
		vkGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo, &topLevelASMemoryRequirements2);

		vk_demo::DVKBuffer* scratchBuffer = vk_demo::DVKBuffer::CreateBuffer(m_VulkanDevice, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, topLevelASMemoryRequirements2.memoryRequirements.size);

		// geometry instance buffer
		std::vector<VkGeometryInstance> geometryInstances(m_Scene.entities.size());
		for (int32 i = 0; i < m_Scene.entities.size(); ++i)
		{
			Matrix4x4 matrix = m_Scene.entities[i]->GetWorldTransform();
			matrix.SetTransposed();

			VkGeometryInstance& geometryInstance = geometryInstances[i];
			memcpy(geometryInstance.transform, &matrix.m, sizeof(float) * 12);

			geometryInstance.instanceId = i;
			geometryInstance.mask = 0xFF;
			geometryInstance.instanceOffset = 0;
			geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
			geometryInstance.accelerationStructureHandle = m_BottomLevelsAS[i].handle;
		}

		vk_demo::DVKBuffer* geometryInstanceBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice,
			VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(VkGeometryInstance) * geometryInstances.size(),
			geometryInstances.data()
		);

		cmdBuffer->Begin();

		vkCmdBuildAccelerationStructureNV(cmdBuffer->cmdBuffer, &accelerationStructureInfo, geometryInstanceBuffer->buffer, 0, VK_FALSE, m_TopLevelAS.accelerationStructure, VK_NULL_HANDLE, scratchBuffer->buffer, 0);

		cmdBuffer->Submit();

		delete scratchBuffer;
		delete geometryInstanceBuffer;
	}

	void CreateBottomLevelAS(vk_demo::DVKCommandBuffer* cmdBuffer)
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		m_BottomLevelsAS.resize(m_Scene.meshes.size());

		for (int32 i = 0; i < m_Scene.meshes.size(); ++i)
		{
			AccelerationStructureInstance& asInstance = m_BottomLevelsAS[i];
			Mesh* mesh = m_Scene.meshes[i];

			VkGeometryNV geometryNV;
			ZeroVulkanStruct(geometryNV, VK_STRUCTURE_TYPE_GEOMETRY_NV);
			geometryNV.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
			geometryNV.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
			geometryNV.geometry.aabbs = {};
			geometryNV.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
			geometryNV.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
			geometryNV.geometry.triangles.vertexData = mesh->vertexBuffer->buffer;
			geometryNV.geometry.triangles.vertexOffset = 0;
			geometryNV.geometry.triangles.vertexCount = mesh->vertexCount;
			geometryNV.geometry.triangles.vertexStride = sizeof(float) * 11;
			geometryNV.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT ;
			geometryNV.geometry.triangles.indexData = mesh->indexBuffer->buffer;
			geometryNV.geometry.triangles.indexOffset = 0;
			geometryNV.geometry.triangles.indexCount = mesh->indexCount;
			geometryNV.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
			geometryNV.geometry.triangles.transformData = VK_NULL_HANDLE;
			geometryNV.geometry.triangles.transformOffset = 0;

			VkAccelerationStructureInfoNV accelerationStructureInfo;
			ZeroVulkanStruct(accelerationStructureInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV);
			accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
			accelerationStructureInfo.geometryCount = 1;
			accelerationStructureInfo.pGeometries = &geometryNV;

			VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo;
			ZeroVulkanStruct(accelerationStructureCreateInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV);
			accelerationStructureCreateInfo.info = accelerationStructureInfo;
			VERIFYVULKANRESULT(vkCreateAccelerationStructureNV(device, &accelerationStructureCreateInfo, VULKAN_CPU_ALLOCATOR, &asInstance.accelerationStructure));

			VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo;
			ZeroVulkanStruct(memoryRequirementsInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV);
			memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
			memoryRequirementsInfo.accelerationStructure = asInstance.accelerationStructure;

			VkMemoryRequirements2 memoryRequirements2;
			vkGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo, &memoryRequirements2);

			uint32 memoryTypeIndex = 0;
			m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);

			VkMemoryAllocateInfo memoryAllocateInfo;
			ZeroVulkanStruct(memoryAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
			memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
			memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
			VERIFYVULKANRESULT(vkAllocateMemory(device, &memoryAllocateInfo, VULKAN_CPU_ALLOCATOR, &asInstance.memory));

			VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo;
			ZeroVulkanStruct(accelerationStructureMemoryInfo, VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV);
			accelerationStructureMemoryInfo.accelerationStructure = asInstance.accelerationStructure;
			accelerationStructureMemoryInfo.memory = asInstance.memory;
			VERIFYVULKANRESULT(vkBindAccelerationStructureMemoryNV(device, 1, &accelerationStructureMemoryInfo));

			VERIFYVULKANRESULT(vkGetAccelerationStructureHandleNV(device, asInstance.accelerationStructure, sizeof(uint64_t), &asInstance.handle));

			// scratch size
			memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
			memoryRequirementsInfo.accelerationStructure = asInstance.accelerationStructure;

			VkMemoryRequirements2 bottomLevelASMemoryRequirements2;
			vkGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo, &bottomLevelASMemoryRequirements2);

			vk_demo::DVKBuffer* scratchBuffer = vk_demo::DVKBuffer::CreateBuffer(m_VulkanDevice, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bottomLevelASMemoryRequirements2.memoryRequirements.size);

			cmdBuffer->Begin();

			vkCmdBuildAccelerationStructureNV(cmdBuffer->cmdBuffer, &accelerationStructureInfo, VK_NULL_HANDLE, 0, VK_FALSE, asInstance.accelerationStructure, VK_NULL_HANDLE, scratchBuffer->buffer, 0);

			cmdBuffer->Submit();

			delete scratchBuffer;
		}
	}

	void DestroyAssets()
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		m_UniformBuffer->UnMap();

		m_Scene.Destroy();

		delete m_Shader;
		delete m_Material;

		delete m_MaterialsBuffer;
		delete m_ObjectsBuffer;
		delete m_StorageImage; 
		delete m_UniformBuffer;
		delete m_ShaderBindingTable;

		for (int32 i = 0; i < m_BottomLevelsAS.size(); ++i)
		{
			vkDestroyAccelerationStructureNV(device, m_BottomLevelsAS[i].accelerationStructure, VULKAN_CPU_ALLOCATOR);
			vkFreeMemory(device, m_BottomLevelsAS[i].memory, VULKAN_CPU_ALLOCATOR);
		}

		vkDestroyAccelerationStructureNV(device, m_TopLevelAS.accelerationStructure, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, m_TopLevelAS.memory, VULKAN_CPU_ALLOCATOR);

		vkDestroyPipeline(device, m_Pipeline, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);

		for (int32 i = 0; i < m_DescriptorSetLayouts.size(); ++i)
		{
			vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayouts[i], VULKAN_CPU_ALLOCATOR);
		}

		vkDestroyDescriptorPool(device, m_DescriptorPool, VULKAN_CPU_ALLOCATOR);
	}

	void SetupGfxCommand(int32 backBufferIndex)
	{
		VkCommandBuffer commandBuffer = m_CommandBuffers[backBufferIndex];

		VkCommandBufferBeginInfo cmdBeginInfo;
		ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

		// raytracing
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_Pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_PipelineLayout, 0, m_DescriptorSets.size(), m_DescriptorSets.data(), 0, 0);

		VkDeviceSize stride = m_RayTracingPropertiesNV.shaderGroupHandleSize;

		vkCmdTraceRaysNV(commandBuffer,
			m_ShaderBindingTable->buffer, stride * 0,
			m_ShaderBindingTable->buffer, stride * 1, stride,
			m_ShaderBindingTable->buffer, stride * 2, stride,
			VK_NULL_HANDLE, 0, 0,
			m_FrameWidth, m_FrameHeight, 1
		);

		// postprocess pass
		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = m_FrameHeight;
		viewport.width = m_FrameWidth;
		viewport.height = -m_FrameHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.extent.width  = m_FrameWidth;
		scissor.extent.height = m_FrameHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;

		VkClearValue clearValues[2];
		clearValues[0].color        = { { 0.0f, 0.0f, 0.0f, 1.0f } };
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

		// ui pass
		m_GUI->BindDrawCmd(commandBuffer, m_RenderPass);

		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
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

	PFN_vkCreateAccelerationStructureNV					vkCreateAccelerationStructureNV;
	PFN_vkDestroyAccelerationStructureNV				vkDestroyAccelerationStructureNV;
	PFN_vkBindAccelerationStructureMemoryNV				vkBindAccelerationStructureMemoryNV;
	PFN_vkGetAccelerationStructureMemoryRequirementsNV	vkGetAccelerationStructureMemoryRequirementsNV;
	PFN_vkGetAccelerationStructureHandleNV				vkGetAccelerationStructureHandleNV;
	PFN_vkCmdBuildAccelerationStructureNV				vkCmdBuildAccelerationStructureNV;
	PFN_vkCreateRayTracingPipelinesNV					vkCreateRayTracingPipelinesNV;
	PFN_vkGetRayTracingShaderGroupHandlesNV				vkGetRayTracingShaderGroupHandlesNV;
	PFN_vkCmdTraceRaysNV								vkCmdTraceRaysNV;

	VkPhysicalDeviceDescriptorIndexingFeatures			m_IndexingFeatures;
	VkPhysicalDeviceFeatures2							m_EnabledFeatures2;
	VkPhysicalDeviceRayTracingPropertiesNV				m_RayTracingPropertiesNV;

	std::vector<AccelerationStructureInstance>			m_BottomLevelsAS;
	AccelerationStructureInstance						m_TopLevelAS;

	vk_demo::DVKBuffer*									m_ShaderBindingTable = nullptr;
	vk_demo::DVKBuffer*									m_UniformBuffer = nullptr;
	CameraParamBlock									m_CameraParam;
	vk_demo::DVKCamera									m_ViewCamera;

	vk_demo::DVKTexture*								m_StorageImage = nullptr;

	VkPipeline											m_Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout									m_PipelineLayout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet>						m_DescriptorSets;
	std::vector<VkDescriptorSetLayout>					m_DescriptorSetLayouts;
	VkDescriptorPool									m_DescriptorPool = VK_NULL_HANDLE;

	vk_demo::DVKModel*									m_Quad = nullptr;
	vk_demo::DVKMaterial*								m_Material = nullptr;
	vk_demo::DVKShader*									m_Shader = nullptr;

	Scene												m_Scene;
	vk_demo::DVKBuffer*									m_MaterialsBuffer = nullptr;
	vk_demo::DVKBuffer*									m_ObjectsBuffer = nullptr;

	bool 												m_Ready = false;

	ImageGUIContext*									m_GUI = nullptr;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<RTXRayTracingSimpleDemo>(1400, 900, "RTXRayTracingSimpleDemo", cmdLine);
}
