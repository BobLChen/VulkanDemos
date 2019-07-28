
#pragma once

#include "Engine.h"
#include "DVKCommand.h"
#include "DVKBuffer.h"
#include "DVKIndexBuffer.h"
#include "DVKVertexBuffer.h"

#include "Common/Common.h"
#include "Math/Math.h"
#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"

#include "Vulkan/VulkanCommon.h"

#include <string>
#include <cstring>
#include <vector>
#include <memory>

struct aiMesh;
struct aiScene;
struct aiNode;

namespace vk_demo
{
    struct DVKNode;
    
    struct DVKBoundingBox
    {
        Vector3 min;
        Vector3 max;
        
        DVKBoundingBox()
            : min(MAX_flt, MAX_flt, MAX_flt)
            , max(MIN_flt, MIN_flt, MIN_flt)
        {
            
        }
        
        DVKBoundingBox(const Vector3& inMin, const Vector3& inMax)
            : min(inMin)
            , max(inMax)
        {
            
        }
    };
    
    struct DVKModelUniformBlock
    {
        Matrix4x4   model;
    };

	struct DVKPrimitive
	{
		DVKIndexBuffer*					indexBuffer = nullptr;
        DVKVertexBuffer*				vertexBuffer = nullptr;

		std::vector<float>				vertices;
		std::vector<uint16>				indices;
        
        int32                           vertexCount = 0;
        int32                           indexCount = 0;

		DVKPrimitive()
		{

		}
        
		~DVKPrimitive()
		{
			if (indexBuffer) {
				delete indexBuffer;
			}
			if (vertexBuffer) {
				delete vertexBuffer;
			}
			indexBuffer  = nullptr;
			vertexBuffer = nullptr;
		}

		void BindDrawCmd(VkCommandBuffer cmdBuffer)
		{
			vertexBuffer->Bind(cmdBuffer);
			indexBuffer->BindDraw(cmdBuffer);
		}
	};

	struct DVKMaterialInfo
	{
		std::string		diffuse;
		std::string		normalmap;
		std::string		specular;
	};
    
    struct DVKMesh
    {
		typedef std::vector<DVKPrimitive*> DVKPrimitives;

		DVKPrimitives	primitives;
		DVKBoundingBox	bounding;
        DVKNode*		linkNode;

		DVKMaterialInfo	material;
        
		int32			vertexCount;
		int32			triangleCount;

        DVKMesh()
            : linkNode(nullptr)
			, vertexCount(0)
			, triangleCount(0)
        {
            
        }

		void BindDrawCmd(VkCommandBuffer cmdBuffer)
		{
			for (int i = 0; i < primitives.size(); ++i) {
				primitives[i]->BindDrawCmd(cmdBuffer);
			}
		}
        
        ~DVKMesh()
        {
			for (int i = 0; i < primitives.size(); ++i) {
				delete primitives[i];
			}
            primitives.clear();
            linkNode = nullptr;
        }
    };
    
    struct DVKNode
    {
        std::string					name;

		std::vector<DVKMesh*>		meshes;

		DVKNode*					parent;
        std::vector<DVKNode*>		children;

        Matrix4x4					localMatrix;
        Matrix4x4					globalMatrix;

		int32						index;
        
        DVKNode()
            : name("None")
            , parent(nullptr)
			, index(-1)
        {
            
        }
        
        const Matrix4x4& GetLocalMatrix()
        {
            return localMatrix;
        }
        
        const Matrix4x4& GetGlobalMatrix()
        {
            globalMatrix = localMatrix;
            
            if (parent) {
                globalMatrix.Append(parent->GetGlobalMatrix());
            }
            return globalMatrix;
        }

		DVKBoundingBox GetBounds()
		{
			DVKBoundingBox bounds;
			bounds.min.Set(0, 0, 0);
			bounds.max.Set(0, 0, 0);

			if (meshes.size() > 0) 
			{
				for (int32 i = 0; i < meshes.size(); ++i)
				{
					const Matrix4x4& matrix = GetGlobalMatrix();
					Vector3 mmin = matrix.TransformPosition(meshes[i]->bounding.min);
					Vector3 mmax = matrix.TransformPosition(meshes[i]->bounding.max);

					bounds.min.x = MMath::Min(bounds.min.x, mmin.x);
					bounds.min.y = MMath::Min(bounds.min.y, mmin.y);
					bounds.min.z = MMath::Min(bounds.min.z, mmin.z);

					bounds.max.x = MMath::Max(bounds.max.x, mmax.x);
					bounds.max.y = MMath::Max(bounds.max.y, mmax.y);
					bounds.max.z = MMath::Max(bounds.max.z, mmax.z);
				}
			}

			for (int32 i = 0; i < children.size(); ++i) 
			{
				DVKBoundingBox childBounds = children[i]->GetBounds();
				bounds.min.x = MMath::Min(bounds.min.x, childBounds.min.x);
				bounds.min.y = MMath::Min(bounds.min.y, childBounds.min.y);
				bounds.min.z = MMath::Min(bounds.min.z, childBounds.min.z);

				bounds.max.x = MMath::Max(bounds.max.x, childBounds.max.x);
				bounds.max.y = MMath::Max(bounds.max.y, childBounds.max.y);
				bounds.max.z = MMath::Max(bounds.max.z, childBounds.max.z);
			}

			return bounds;
		}
        
        ~DVKNode()
        {
			for (int32 i = 0; i < meshes.size(); ++i) {
				delete meshes[i];
			}
			meshes.clear();
            
            for (int32 i = 0; i < children.size(); ++i) {
                delete children[i];
            }
            children.clear();
        }
    };
    
    class DVKModel
    {
    private:
        DVKModel()
			: device(nullptr)
			, rootNode(nullptr)
        {
            
        }
        
    public:
        ~DVKModel()
        {
            delete rootNode;
			rootNode = nullptr;
			device = nullptr;

			meshes.clear();
			linearNodes.clear();
        }

		VkVertexInputBindingDescription GetInputBinding();

		std::vector<VkVertexInputAttributeDescription> GetInputAttributes();
        
        static DVKModel* LoadFromFile(const std::string& filename, std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, const std::vector<VertexAttribute>& attributes);
        
        static DVKModel* Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, const std::vector<float>& vertices, const std::vector<uint16>& indices, const std::vector<VertexAttribute>& attributes);
        
    protected:
        
        DVKNode* LoadNode(const aiNode* node, const aiScene* scene);
        
		DVKMesh* LoadMesh(const aiMesh* mesh, const aiScene* scene);

    public:
        
        std::shared_ptr<VulkanDevice>	device;
        
        DVKNode*						rootNode;
        std::vector<DVKNode*>			linearNodes;
        std::vector<DVKMesh*>			meshes;

		std::vector<VertexAttribute>	attributes;

	private:

		DVKCommandBuffer*				cmdBuffer;
    };
    
};
