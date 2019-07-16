
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

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

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
    
    struct DVKMesh
    {
        DVKIndexBuffer*					indexBuffer;
        DVKVertexBuffer*				vertexBuffer;

		std::vector<float>				vertices;
		std::vector<uint16>				indices;

		DVKBoundingBox					bounding;

        DVKNode*						linkNode;

        DVKMesh()
            : indexBuffer(nullptr)
			, vertexBuffer(nullptr)
            , linkNode(nullptr)
        {
            
        }

		void BindDrawCmd(VkCommandBuffer cmdBuffer)
		{
			vertexBuffer->Bind(cmdBuffer);
			indexBuffer->BindDraw(cmdBuffer);
		}
        
        ~DVKMesh()
        {
			if (vertexBuffer) {
				delete vertexBuffer;
				vertexBuffer = nullptr;
			}

            if (indexBuffer) {
				delete indexBuffer;
				indexBuffer = nullptr;
			}
            
            linkNode = nullptr;
        }
    };
    
    struct DVKNode
    {
        std::string					name;

		DVKMesh*					mesh;

		DVKNode*					parent;
        std::vector<DVKNode*>		children;

        Matrix4x4					localMatrix;
        Matrix4x4					globalMatrix;

		int32						index;
        bool						invalid;
        
        DVKNode()
            : name("None")
			, mesh(nullptr)
            , parent(nullptr)
			, index(-1)
            , invalid(true)
        {
            
        }
        
        const Matrix4x4& GetLocalMatrix()
        {
            return localMatrix;
        }
        
        const Matrix4x4& GetGlobalMatrix()
        {
            if (!invalid) {
                return globalMatrix;
            }
            invalid = false;
            
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

			if (mesh) {
				const Matrix4x4& matrix = GetGlobalMatrix();
				bounds.min = matrix.TransformPosition(mesh->bounding.min);
				bounds.max = matrix.TransformPosition(mesh->bounding.max);
			}

			for (int32 i = 0; i < children.size(); ++i) {
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
            if (mesh) {
                delete mesh;
                mesh = nullptr;
            }
            
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

		std::vector<VkVertexInputAttributeDescription> GetInputAttributes(const std::vector<VertexAttribute>& shaderInputs);
        
        static DVKModel* LoadFromFile(const std::string& filename, std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, const std::vector<VertexAttribute>& attributes);
        
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
