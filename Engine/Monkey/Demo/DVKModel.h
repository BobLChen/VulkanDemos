
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

namespace vk_demo
{
    struct Node;
    
    struct BoundingBox
    {
        Vector3 min;
        Vector3 max;
        
        BoundingBox()
            : min(MAX_flt, MAX_flt, MAX_flt)
            , max(MIN_flt, MIN_flt, MIN_flt)
        {
            
        }
        
        BoundingBox(const Vector3& inMin, const Vector3& inMax)
            : min(inMin)
            , max(inMax)
        {
            
        }
    };
    
    struct SubMesh
    {
        uint32 firstIndex;
        uint32 indexCount;
        uint32 vertexCount;
        
        BoundingBox bounding;
        
        SubMesh(uint32 inFirstIndex, uint32 inIndexCount, uint32 inVertexCount)
            : firstIndex(inFirstIndex)
            , indexCount(inIndexCount)
            , vertexCount(inVertexCount)
        {
            
        }
    };
    
    struct MeshUniformBlock
    {
        Matrix4x4   model;
    };
    
    struct Mesh
    {
        std::vector<SubMesh*>   subMeshes;
        BoundingBox             bounding;
        DVKBuffer*              modelBuffer;
        DVKIndexBuffer*         indexBuffer;
        DVKVertexBuffer*        vertexBuffer;
        Node*                   linkNode;
        
        Mesh()
            : modelBuffer(nullptr)
            , linkNode(nullptr)
        {
            
        }
        
        ~Mesh()
        {
            if (modelBuffer) {
                delete modelBuffer;
            }
            modelBuffer = nullptr;
            
            linkNode = nullptr;
        }
    };
    
    struct Node
    {
        std::string         name;
        int32               index;
        Node*               parent;
        std::vector<Node*>  children;
        Matrix4x4           localMatrix;
        Matrix4x4           globalMatrix;
        BoundingBox         bounds;
        Mesh*               mesh;
        bool                invalid;
        
        Node()
            : name("None")
            , parent(nullptr)
            , mesh(nullptr)
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
        
        ~Node()
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
        {
            
        }
        
    public:
        ~DVKModel()
        {
            
        }
        
    public:
        
        std::vector<Node*>      nodes;
        std::vector<Node*>      linearNodes;
        std::vector<Mesh*>      meshes;
        
    };
    
};
