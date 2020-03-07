#include "Common/Common.h"
#include "Common/Log.h"

#include "Demo/DVKCommon.h"

#include "Math/Vector4.h"
#include "Math/Matrix4x4.h"

#include <vector>
#include <thread>

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

// static_assert(sizeof(VkGeometryInstance) == 64, "VkGeometryInstance structure compiles to incorrect size");

struct CameraParamBlock
{
	Vector4 lens;
	Vector4 pos;
	Matrix4x4 invProj;
	Matrix4x4 invView;
};

class RTXRayTracingStartBasic : public DemoBase
{
public:
	RTXRayTracingStartBasic(int32 width, int32 height, const char* title, const std::vector<std::string>& cmdLine)
		: DemoBase(width, height, title, cmdLine)
	{
		deviceExtensions.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
		instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		deviceExtensions.push_back(VK_NV_RAY_TRACING_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME);
		instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		ZeroVulkanStruct(m_EnabledFeatures2, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2);
		m_EnabledFeatures2.pNext = nullptr;

		physicalDeviceFeatures = &m_EnabledFeatures2;
	}

	virtual ~RTXRayTracingStartBasic()
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

		m_ViewCamera.Update(time, delta);

		UpdateUniformBuffer();
		SetupGfxCommand(bufferIndex);

		DemoBase::Present(bufferIndex);
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

		m_CameraParam.invProj = m_ViewCamera.GetProjection();
		m_CameraParam.invProj.SetInverse();
		m_CameraParam.invView = m_ViewCamera.GetView();
		m_CameraParam.invView.SetInverse();

		memcpy(m_UniformBuffer->mapped, &m_CameraParam, sizeof(CameraParamBlock));

	}

	void PrepareDescriptorSets()
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		std::vector<VkDescriptorPoolSize> poolSizes(3);
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSizes[1].descriptorCount = 1;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[2].descriptorCount = 1;
		
		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo;
		ZeroVulkanStruct(descriptorPoolCreateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
		descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
		descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
		descriptorPoolCreateInfo.maxSets = 1;
		VERIFYVULKANRESULT(vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorPool));

		VkDescriptorSetAllocateInfo descriptorSetAllocateInfo;
		ZeroVulkanStruct(descriptorSetAllocateInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
		descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool;
		descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayout;
		descriptorSetAllocateInfo.descriptorSetCount = 1;
		VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &m_DescriptorSet));

		VkWriteDescriptorSetAccelerationStructureNV descriptorASNV;
		ZeroVulkanStruct(descriptorASNV, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV);
		descriptorASNV.accelerationStructureCount = 1;
		descriptorASNV.pAccelerationStructures = &m_TopLevelAS;

		VkWriteDescriptorSet asWriteDescriptorSet;
		ZeroVulkanStruct(asWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		asWriteDescriptorSet.pNext = &descriptorASNV;
		asWriteDescriptorSet.dstSet = m_DescriptorSet;
		asWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
		asWriteDescriptorSet.dstBinding = 0;
		asWriteDescriptorSet.descriptorCount = 1;
		
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageView = m_StorageImage->imageView;
		imageInfo.imageLayout = m_StorageImage->imageLayout;

		VkWriteDescriptorSet imageWriteDescriptorSet;
		ZeroVulkanStruct(imageWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		imageWriteDescriptorSet.pImageInfo = &imageInfo;
		imageWriteDescriptorSet.dstSet = m_DescriptorSet;
		imageWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageWriteDescriptorSet.dstBinding = 1;
		imageWriteDescriptorSet.descriptorCount = 1;

		VkWriteDescriptorSet uboWriteDescriptorSet;
		ZeroVulkanStruct(uboWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
		uboWriteDescriptorSet.pBufferInfo = &m_UniformBuffer->descriptor;
		uboWriteDescriptorSet.dstSet = m_DescriptorSet;
		uboWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboWriteDescriptorSet.dstBinding = 2;
		uboWriteDescriptorSet.descriptorCount = 1;
		
		std::vector<VkWriteDescriptorSet> writeDescriptorSets;
		writeDescriptorSets.push_back(asWriteDescriptorSet);
		writeDescriptorSets.push_back(imageWriteDescriptorSet);
		writeDescriptorSets.push_back(uboWriteDescriptorSet);

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

		VkDescriptorSetLayoutBinding asLayoutBinding = {};
		asLayoutBinding.binding = 0;
		asLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
		asLayoutBinding.descriptorCount = 1;
		asLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;

		VkDescriptorSetLayoutBinding imageLayoutBinding = {};
		imageLayoutBinding.binding = 1;
		imageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageLayoutBinding.descriptorCount = 1;
		imageLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_NV;

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
		VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &layoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_DescriptorSetLayout));

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
		ZeroVulkanStruct(pipelineLayoutCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
		pipelineLayoutCreateInfo.setLayoutCount = 1;
		pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
		VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, VULKAN_CPU_ALLOCATOR, &m_PipelineLayout));

		auto rayGenShaderModule = vk_demo::DVKShaderModule::Create(m_VulkanDevice, "assets/shaders/62_RTXRayTracingBasic/raygen.rgen.spv", VK_SHADER_STAGE_RAYGEN_BIT_NV);
		auto rayMisShaderModule = vk_demo::DVKShaderModule::Create(m_VulkanDevice, "assets/shaders/62_RTXRayTracingBasic/miss.rmiss.spv", VK_SHADER_STAGE_MISS_BIT_NV);
		auto rayHitShaderModule = vk_demo::DVKShaderModule::Create(m_VulkanDevice, "assets/shaders/62_RTXRayTracingBasic/closesthit.rchit.spv", VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV);

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
		pipelineCreateInfo.maxRecursionDepth = 1;
		pipelineCreateInfo.layout = m_PipelineLayout;
		VERIFYVULKANRESULT(vkCreateRayTracingPipelinesNV(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Pipeline));

		delete rayGenShaderModule;
		delete rayMisShaderModule;
		delete rayHitShaderModule;
	}

	void PrepareUniformBuffers()
	{
		m_UniformBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice, 
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(CameraParamBlock),
			&m_CameraParam
		);
		m_UniformBuffer->Map();

		m_ViewCamera.SetPosition(0, 0, -5.0f);
		m_ViewCamera.Perspective(PI / 4, (float)GetWidth(), (float)GetHeight(), 0.1f, 1000.0f);
	}

	void LoadAssets()
	{
		vk_demo::DVKCommandBuffer* cmdBuffer = vk_demo::DVKCommandBuffer::Create(m_VulkanDevice, m_CommandPool);

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
			"assets/shaders/62_RTXRayTracingBasic/result.vert.spv",
			"assets/shaders/62_RTXRayTracingBasic/result.frag.spv"
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
		
		// geometry
		std::vector<float> vertices = { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 0.0f };
		std::vector<uint16> indices = { 0, 1, 2 };

		m_VertexBuffer = vk_demo::DVKVertexBuffer::Create(m_VulkanDevice, cmdBuffer, vertices, { VertexAttribute::VA_Position });
		m_IndexBuffer  = vk_demo::DVKIndexBuffer::Create(m_VulkanDevice, cmdBuffer, indices);

		// geometry info
		VkGeometryNV geometryNV;
		ZeroVulkanStruct(geometryNV, VK_STRUCTURE_TYPE_GEOMETRY_NV);
		geometryNV.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
		geometryNV.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
		geometryNV.geometry.aabbs = {};
		geometryNV.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
		geometryNV.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
		geometryNV.geometry.triangles.vertexData = m_VertexBuffer->dvkBuffer->buffer;
		geometryNV.geometry.triangles.vertexOffset = 0;
		geometryNV.geometry.triangles.vertexCount = vertices.size();
		geometryNV.geometry.triangles.vertexStride = sizeof(float) * 3;
		geometryNV.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
		geometryNV.geometry.triangles.indexData = m_IndexBuffer->dvkBuffer->buffer;
		geometryNV.geometry.triangles.indexOffset = 0;
		geometryNV.geometry.triangles.indexCount = indices.size();
		geometryNV.geometry.triangles.indexType = VK_INDEX_TYPE_UINT16;
		geometryNV.geometry.triangles.transformData = VK_NULL_HANDLE;
		geometryNV.geometry.triangles.transformOffset = 0;
		
		// bottom level
		CreateBottomLevelAS(cmdBuffer, &geometryNV, 1);

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
		accelerationStructureInfo.instanceCount = 1;
		accelerationStructureInfo.geometryCount = 0;

		VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo;
		ZeroVulkanStruct(accelerationStructureCreateInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV);
		accelerationStructureCreateInfo.info = accelerationStructureInfo;
		VERIFYVULKANRESULT(vkCreateAccelerationStructureNV(device, &accelerationStructureCreateInfo, VULKAN_CPU_ALLOCATOR, &m_TopLevelAS));

		VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo;
		ZeroVulkanStruct(memoryRequirementsInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV);
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
		memoryRequirementsInfo.accelerationStructure = m_TopLevelAS;

		VkMemoryRequirements2 memoryRequirements2;
		vkGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo, &memoryRequirements2);

		uint32 memoryTypeIndex = 0;
		m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		VkMemoryAllocateInfo memoryAllocateInfo;
		ZeroVulkanStruct(memoryAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memoryAllocateInfo, VULKAN_CPU_ALLOCATOR, &m_TopLevelMemory));

		VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo;
		ZeroVulkanStruct(accelerationStructureMemoryInfo, VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV);
		accelerationStructureMemoryInfo.accelerationStructure = m_TopLevelAS;
		accelerationStructureMemoryInfo.memory = m_TopLevelMemory;
		VERIFYVULKANRESULT(vkBindAccelerationStructureMemoryNV(device, 1, &accelerationStructureMemoryInfo));

		VERIFYVULKANRESULT(vkGetAccelerationStructureHandleNV(device, m_TopLevelAS, sizeof(uint64_t), &m_TopLevelHandle));

		// scratch size
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
		memoryRequirementsInfo.accelerationStructure = m_TopLevelAS;

		VkMemoryRequirements2 topLevelASMemoryRequirements2;
		vkGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo, &topLevelASMemoryRequirements2);

		vk_demo::DVKBuffer* scratchBuffer = vk_demo::DVKBuffer::CreateBuffer(m_VulkanDevice, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, topLevelASMemoryRequirements2.memoryRequirements.size);

		// geometry instance buffer
		cmdBuffer->Begin();

		std::vector<float> transform = {
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
		};

		VkGeometryInstance geometryInstance;
		memcpy(geometryInstance.transform, transform.data(), sizeof(float) * 12);
		geometryInstance.instanceId = 0;
		geometryInstance.mask = 0xFF;
		geometryInstance.instanceOffset = 0;
		geometryInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_CULL_DISABLE_BIT_NV;
		geometryInstance.accelerationStructureHandle = m_BottomLevelHandle;

		vk_demo::DVKBuffer* geometryInstanceBuffer = vk_demo::DVKBuffer::CreateBuffer(
			m_VulkanDevice,
			VK_BUFFER_USAGE_RAY_TRACING_BIT_NV,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(VkGeometryInstance),
			&geometryInstance
		);

		vkCmdBuildAccelerationStructureNV(cmdBuffer->cmdBuffer, &accelerationStructureInfo, geometryInstanceBuffer->buffer, 0, VK_FALSE, m_TopLevelAS, VK_NULL_HANDLE, scratchBuffer->buffer, 0);

		VkMemoryBarrier memoryBarrier;
		ZeroVulkanStruct(memoryBarrier, VK_STRUCTURE_TYPE_MEMORY_BARRIER);
		memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
		memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
		vkCmdPipelineBarrier(cmdBuffer->cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

		cmdBuffer->Submit();

		delete scratchBuffer;
		delete geometryInstanceBuffer;
	}

	void CreateBottomLevelAS(vk_demo::DVKCommandBuffer* cmdBuffer, const VkGeometryNV* geometries, int32 geometryCount)
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		VkAccelerationStructureInfoNV accelerationStructureInfo;
		ZeroVulkanStruct(accelerationStructureInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV);
		accelerationStructureInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
		accelerationStructureInfo.instanceCount = 0;
		accelerationStructureInfo.geometryCount = geometryCount;
		accelerationStructureInfo.pGeometries = geometries;

		VkAccelerationStructureCreateInfoNV accelerationStructureCreateInfo;
		ZeroVulkanStruct(accelerationStructureCreateInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV);
		accelerationStructureCreateInfo.info = accelerationStructureInfo;
		VERIFYVULKANRESULT(vkCreateAccelerationStructureNV(device, &accelerationStructureCreateInfo, VULKAN_CPU_ALLOCATOR, &m_BottomLevelAS));

		VkAccelerationStructureMemoryRequirementsInfoNV memoryRequirementsInfo;
		ZeroVulkanStruct(memoryRequirementsInfo, VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV);
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
		memoryRequirementsInfo.accelerationStructure = m_BottomLevelAS;

		VkMemoryRequirements2 memoryRequirements2;
		vkGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo, &memoryRequirements2);

		uint32 memoryTypeIndex = 0;
		m_VulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memoryRequirements2.memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);

		VkMemoryAllocateInfo memoryAllocateInfo;
		ZeroVulkanStruct(memoryAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		memoryAllocateInfo.allocationSize = memoryRequirements2.memoryRequirements.size;
		memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(device, &memoryAllocateInfo, VULKAN_CPU_ALLOCATOR, &m_BottomLevelMemory));

		VkBindAccelerationStructureMemoryInfoNV accelerationStructureMemoryInfo;
		ZeroVulkanStruct(accelerationStructureMemoryInfo, VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV);
		accelerationStructureMemoryInfo.accelerationStructure = m_BottomLevelAS;
		accelerationStructureMemoryInfo.memory = m_BottomLevelMemory;
		VERIFYVULKANRESULT(vkBindAccelerationStructureMemoryNV(device, 1, &accelerationStructureMemoryInfo));

		VERIFYVULKANRESULT(vkGetAccelerationStructureHandleNV(device, m_BottomLevelAS, sizeof(uint64_t), &m_BottomLevelHandle));

		// scratch size
		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
		memoryRequirementsInfo.accelerationStructure = m_BottomLevelAS;
		VkMemoryRequirements2 bottomLevelASMemoryRequirements2;
		vkGetAccelerationStructureMemoryRequirementsNV(device, &memoryRequirementsInfo, &bottomLevelASMemoryRequirements2);

		vk_demo::DVKBuffer* scratchBuffer = vk_demo::DVKBuffer::CreateBuffer(m_VulkanDevice, VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bottomLevelASMemoryRequirements2.memoryRequirements.size);

		// begin cmd buffer
		cmdBuffer->Begin();

		vkCmdBuildAccelerationStructureNV(cmdBuffer->cmdBuffer, &accelerationStructureInfo, VK_NULL_HANDLE, 0, VK_FALSE, m_BottomLevelAS, VK_NULL_HANDLE, scratchBuffer->buffer, 0);

		// insert memory barrier
		VkMemoryBarrier memoryBarrier;
		ZeroVulkanStruct(memoryBarrier, VK_STRUCTURE_TYPE_MEMORY_BARRIER);
		memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
		memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
		vkCmdPipelineBarrier(cmdBuffer->cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, 0, 0, 0);

		cmdBuffer->Submit();

		delete scratchBuffer;
	}

	void DestroyAssets()
	{
		VkDevice device = m_VulkanDevice->GetInstanceHandle();

		m_UniformBuffer->UnMap();

		delete m_Shader;
		delete m_Material;

		delete m_IndexBuffer;
		delete m_VertexBuffer;
		delete m_StorageImage; 
		delete m_UniformBuffer;
		delete m_ShaderBindingTable;

		vkDestroyAccelerationStructureNV(device, m_BottomLevelAS, VULKAN_CPU_ALLOCATOR);
		vkDestroyAccelerationStructureNV(device, m_TopLevelAS, VULKAN_CPU_ALLOCATOR);

		vkFreeMemory(device, m_BottomLevelMemory, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(device, m_TopLevelMemory, VULKAN_CPU_ALLOCATOR);

		vkDestroyPipeline(device, m_Pipeline, VULKAN_CPU_ALLOCATOR);
		vkDestroyPipelineLayout(device, m_PipelineLayout, VULKAN_CPU_ALLOCATOR);
		vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayout, VULKAN_CPU_ALLOCATOR);
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
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_PipelineLayout, 0, 1, &m_DescriptorSet, 0, 0);

		VkDeviceSize stride = m_RayTracingPropertiesNV.shaderGroupHandleSize;
		
		vkCmdTraceRaysNV(commandBuffer,
			m_ShaderBindingTable->buffer, stride * 0,
			m_ShaderBindingTable->buffer, stride * 1, stride,
			m_ShaderBindingTable->buffer, stride * 2, stride,
			VK_NULL_HANDLE, 0, 0,
			m_FrameWidth, m_FrameHeight, 1
		);

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

		vkCmdEndRenderPass(commandBuffer);
		VERIFYVULKANRESULT(vkEndCommandBuffer(commandBuffer));
	}

private:

	VkPhysicalDeviceFeatures2							m_EnabledFeatures2;

	PFN_vkCreateAccelerationStructureNV					vkCreateAccelerationStructureNV;
	PFN_vkDestroyAccelerationStructureNV				vkDestroyAccelerationStructureNV;
	PFN_vkBindAccelerationStructureMemoryNV				vkBindAccelerationStructureMemoryNV;
	PFN_vkGetAccelerationStructureMemoryRequirementsNV	vkGetAccelerationStructureMemoryRequirementsNV;
	PFN_vkGetAccelerationStructureHandleNV				vkGetAccelerationStructureHandleNV;
	PFN_vkCmdBuildAccelerationStructureNV				vkCmdBuildAccelerationStructureNV;
	PFN_vkCreateRayTracingPipelinesNV					vkCreateRayTracingPipelinesNV;
	PFN_vkGetRayTracingShaderGroupHandlesNV				vkGetRayTracingShaderGroupHandlesNV;
	PFN_vkCmdTraceRaysNV								vkCmdTraceRaysNV;

	VkPhysicalDeviceRayTracingPropertiesNV				m_RayTracingPropertiesNV;

	VkDeviceMemory										m_BottomLevelMemory = VK_NULL_HANDLE;
	VkAccelerationStructureNV							m_BottomLevelAS = VK_NULL_HANDLE;
	uint64												m_BottomLevelHandle = 0;

	VkDeviceMemory										m_TopLevelMemory = VK_NULL_HANDLE;
	VkAccelerationStructureNV							m_TopLevelAS = VK_NULL_HANDLE;
	uint64												m_TopLevelHandle = 0;

	vk_demo::DVKBuffer*									m_ShaderBindingTable = nullptr;
	vk_demo::DVKBuffer*									m_UniformBuffer = nullptr;
	CameraParamBlock									m_CameraParam;
	vk_demo::DVKCamera									m_ViewCamera;

	vk_demo::DVKVertexBuffer*							m_VertexBuffer = nullptr;
	vk_demo::DVKIndexBuffer*							m_IndexBuffer = nullptr;
	uint32												m_IndexCount = 0;

	vk_demo::DVKTexture*								m_StorageImage = nullptr;

	VkPipeline											m_Pipeline = VK_NULL_HANDLE;
	VkPipelineLayout									m_PipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSet										m_DescriptorSet = VK_NULL_HANDLE;
	VkDescriptorSetLayout								m_DescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool									m_DescriptorPool = VK_NULL_HANDLE;

	vk_demo::DVKModel*									m_Quad = nullptr;
	vk_demo::DVKMaterial*								m_Material = nullptr;
	vk_demo::DVKShader*									m_Shader = nullptr;

	bool 												m_Ready = false;
};

std::shared_ptr<AppModuleBase> CreateAppMode(const std::vector<std::string>& cmdLine)
{
	return std::make_shared<RTXRayTracingStartBasic>(1400, 900, "RTXRayTracingStartBasic", cmdLine);
}
