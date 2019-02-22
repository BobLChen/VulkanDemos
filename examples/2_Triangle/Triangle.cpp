#include "Common/Common.h"
#include "Common/Log.h"
#include "Application/AppModeBase.h"
#include "Vulkan/VulkanPlatform.h"
#include "Vulkan/VulkanDevice.h"
#include "Vulkan/VulkanQueue.h"
#include <vector>

class TriangleMode : public AppModeBase
{
public:
	TriangleMode(int width, int height, const char* title)
		: AppModeBase(width, height, title)
	{

	}

	virtual ~TriangleMode()
	{

	}

	virtual void PreInit() override
	{
		
	}

	virtual void Init() override
	{
		m_VulkanRHI = GetVulkanRHI();
		m_Device    = m_VulkanRHI->GetDevice()->GetInstanceHandle();

		CreateFences();
		CreateMeshBuffers();
		CreateUniformBuffers();
	}

	virtual void Loop() override
	{

	}

	virtual void Exist() override
	{
		DestroyUniformBuffers();
		DestroyMeshBuffers();
		DestroyFences();
	}

private:

	struct VertexBuffer
	{
		VkDeviceMemory memory;
		VkBuffer buffer;
	};

	typedef VertexBuffer IndexBuffer;

	struct Vertex
	{
		float position[3];
		float color[3];
	};

	struct UboVS
	{
		
	};

	void CreateUniformBuffers()
	{
		VkMemoryRequirements memReqInfo;

		VkBufferCreateInfo bufferInfo;
		ZeroVulkanStruct(bufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);

	}

	void DestroyUniformBuffers()
	{

	}

	void CreateMeshBuffers()
	{
		// 顶点数据
		std::vector<Vertex> vertices = {
			{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
			{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
			{ {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
		};

		// 索引数据
		std::vector<uint16> indices = { 0, 1, 2 };
		m_IndicesCount = indices.size();

		// 顶点数据以及索引数据在整个生命周期中几乎不会发生改变，因此最佳的方式是将这些数据存储到GPU的内存中。
		// 存储到GPU内存也能加快GPU的访问。为了存储到GPU内存中，需要如下几个步骤。
		// 1、在主机端(Host)创建一个Buffer
		// 2、将数据拷贝至该Buffer
		// 3、在GPU端(Local Device)创建一个Buffer
		// 4、通过Transfer簇将数据从主机端拷贝至GPU端
		// 5、删除主基端(Host)的Buffer
		// 6、使用GPU端(Local Device)的Buffer进行渲染
		VertexBuffer tempVertexBuffer;
		IndexBuffer tempIndexBuffer;

		void* dataPtr = nullptr;
		VkMemoryRequirements memReqInfo;
		VkMemoryAllocateInfo memAllocInfo;
		ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

		// vertex buffer
		VkBufferCreateInfo vertexBufferInfo;
		ZeroVulkanStruct(vertexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		vertexBufferInfo.size  = vertices.size() * sizeof(Vertex);
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.buffer));

		vkGetBufferMemoryRequirements(m_Device, tempVertexBuffer.buffer, &memReqInfo);
		uint32 memoryTypeIndex = 0;
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempVertexBuffer.memory));
		
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, tempVertexBuffer.buffer, tempVertexBuffer.memory, 0));

		VERIFYVULKANRESULT(vkMapMemory(m_Device, tempVertexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, vertices.data(), vertexBufferInfo.size);
		vkUnmapMemory(m_Device, tempVertexBuffer.memory);

		// local device vertex buffer
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &vertexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.buffer));

		vkGetBufferMemoryRequirements(m_Device, m_VertexBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_VertexBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_VertexBuffer.buffer, m_VertexBuffer.memory, 0));

		// index buffer
		VkBufferCreateInfo indexBufferInfo;
		ZeroVulkanStruct(indexBufferInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
		indexBufferInfo.size  = m_IndicesCount * sizeof(uint16);
		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.buffer));

		vkGetBufferMemoryRequirements(m_Device, tempIndexBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &tempIndexBuffer.memory));
		
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, tempIndexBuffer.buffer, tempIndexBuffer.memory, 0));

		VERIFYVULKANRESULT(vkMapMemory(m_Device, tempIndexBuffer.memory, 0, memAllocInfo.allocationSize, 0, &dataPtr));
		std::memcpy(dataPtr, indices.data(), indexBufferInfo.size);
		vkUnmapMemory(m_Device, tempIndexBuffer.memory);
		
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VERIFYVULKANRESULT(vkCreateBuffer(m_Device, &indexBufferInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.buffer));
		
		vkGetBufferMemoryRequirements(m_Device, m_IndicesBuffer.buffer, &memReqInfo);
		GetVulkanRHI()->GetDevice()->GetMemoryManager().GetMemoryTypeFromProperties(memReqInfo.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex);
		memAllocInfo.allocationSize  = memReqInfo.size;
		memAllocInfo.memoryTypeIndex = memoryTypeIndex;
		VERIFYVULKANRESULT(vkAllocateMemory(m_Device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &m_IndicesBuffer.memory));
		VERIFYVULKANRESULT(vkBindBufferMemory(m_Device, m_IndicesBuffer.buffer, m_IndicesBuffer.memory, 0));

		VkCommandBuffer xferCmdBuffer;
		// gfx queue自带transfer功能，为了优化需要使用转悠的xfer queue。这里为了简单，先将就用。
		VkCommandBufferAllocateInfo xferCmdBufferInfo;
		ZeroVulkanStruct(xferCmdBufferInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
		xferCmdBufferInfo.commandPool = m_VulkanRHI->GetCommandPool();
		xferCmdBufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		xferCmdBufferInfo.commandBufferCount = 1;
		VERIFYVULKANRESULT(vkAllocateCommandBuffers(m_Device, &xferCmdBufferInfo, &xferCmdBuffer));

		// 开始录制命令
		VkCommandBufferBeginInfo cmdBufferBeginInfo;
		ZeroVulkanStruct(cmdBufferBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
		VERIFYVULKANRESULT(vkBeginCommandBuffer(xferCmdBuffer, &cmdBufferBeginInfo));
		
		VkBufferCopy copyRegion = {};
		copyRegion.size = vertices.size() * sizeof(Vertex);
		vkCmdCopyBuffer(xferCmdBuffer, tempVertexBuffer.buffer, m_VertexBuffer.buffer, 1, &copyRegion);
		
		copyRegion.size = indices.size() * sizeof(uint16);
		vkCmdCopyBuffer(xferCmdBuffer, tempIndexBuffer.buffer, m_IndicesBuffer.buffer, 1, &copyRegion);

		// 结束录制
		VERIFYVULKANRESULT(vkEndCommandBuffer(xferCmdBuffer));
		
		// 提交命令，并且等待命令执行完毕。
		VkSubmitInfo submitInfo;
		ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &xferCmdBuffer;

		VkFenceCreateInfo fenceInfo = {};
		ZeroVulkanStruct(fenceInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceInfo.flags = 0;

		VkFence fence;
		VERIFYVULKANRESULT(vkCreateFence(m_Device, &fenceInfo, VULKAN_CPU_ALLOCATOR, &fence));
		VERIFYVULKANRESULT(vkQueueSubmit(m_VulkanRHI->GetDevice()->GetGraphicsQueue()->GetHandle(), 1, &submitInfo, fence));
		VERIFYVULKANRESULT(vkWaitForFences(m_Device, 1, &fence, VK_TRUE, MAX_int64));

		vkDestroyFence(m_Device, fence, VULKAN_CPU_ALLOCATOR);
		vkFreeCommandBuffers(m_Device, m_VulkanRHI->GetCommandPool(), 1, &xferCmdBuffer);

		vkDestroyBuffer(m_Device, tempVertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, tempVertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(m_Device, tempIndexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, tempIndexBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void DestroyMeshBuffers()
	{
		vkDestroyBuffer(m_Device, m_VertexBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, m_VertexBuffer.memory, VULKAN_CPU_ALLOCATOR);
		vkDestroyBuffer(m_Device, m_IndicesBuffer.buffer, VULKAN_CPU_ALLOCATOR);
		vkFreeMemory(m_Device, m_IndicesBuffer.memory, VULKAN_CPU_ALLOCATOR);
	}

	void CreateFences()
	{
		m_Fences.resize(m_VulkanRHI->GetCommandBuffers().size());

		VkFenceCreateInfo fenceCreateInfo;
		ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (int i = 0; i < m_Fences.size(); ++i) 
		{
			VERIFYVULKANRESULT(vkCreateFence(m_Device, &fenceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Fences[i]));
		}
	}

	void DestroyFences()
	{
		for (int i = 0; i < m_Fences.size(); ++i)
		{
			vkDestroyFence(m_Device, m_Fences[i], VULKAN_CPU_ALLOCATOR);
		}
	}

	std::vector<VkFence> m_Fences;
	VkDevice m_Device;
	std::shared_ptr<VulkanRHI> m_VulkanRHI;
	VertexBuffer m_VertexBuffer;
	IndexBuffer m_IndicesBuffer;
	uint32 m_IndicesCount;
};

AppModeBase* CreateAppMode(const char* cmdLine, int32 cmdShow)
{
	return new TriangleMode(800, 600, "Triangle");
}