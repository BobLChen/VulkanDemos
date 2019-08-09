
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
#include "Math/Quat.h"

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
    
	struct DVKPrimitive
	{
		DVKIndexBuffer*		indexBuffer = nullptr;
        DVKVertexBuffer*	vertexBuffer = nullptr;

		std::vector<float>	vertices;
		std::vector<uint16>	indices;
        
        int32               vertexCount = 0;
        int32               indexCount = 0;

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
    
    struct DVKBone
    {
		std::string     name;
        int32           index = -1;
        int32           parent = -1;
        Matrix4x4       inverseBindPose;
		Matrix4x4		finalTransform;
    };

	struct DVKVertexSkin
	{
		int32  used = 0;
		int32  indices[4];
		float  weights[4];
	};

	template<class ValueType>
	struct DVKAnimChannel
	{
		std::vector<float>	   keys;
		std::vector<ValueType> values;

		void GetValue(float key, ValueType& outPrevValue, ValueType& outNextValue, float& outAlpha)
		{
			outAlpha = 0.0f;

			if (keys.size() == 0) {
				return;
			}

			if (key <= keys.front()) {
				outPrevValue = values.front();
				outNextValue = values.front();
				outAlpha     = 0.0f;
				return;
			}

			if (key >= keys.back()) {
				outPrevValue = values.back();
				outNextValue = values.back();
				outAlpha     = 0.0f;
				return;
			}

			int32 frameIndex = 0;
			for (int32 i = 0; i < keys.size(); ++i) {
				if (key <= keys[i]) {
					frameIndex = i;
					break;
				}
			}
            
			outPrevValue = values[frameIndex - 1];
			outNextValue = values[frameIndex];

			float prevKey = keys[frameIndex - 1];
			float nextKey = keys[frameIndex];
			outAlpha      = (key - prevKey) / (nextKey - prevKey);
		}
	};

	struct DVKAnimationClip
	{
		std::string					nodeName;
		float						duration;
		DVKAnimChannel<Vector3>		positions;
		DVKAnimChannel<Vector3>		scales;
		DVKAnimChannel<Quat>		rotations;
	};

	struct DVKAnimation
	{
		std::string name;
		float		time = 0.0f;
		float       duration = 0.0f;
		float		speed = 1.0f;
		std::unordered_map<std::string, DVKAnimationClip> clips;
	};
    
    struct DVKMesh
    {
		typedef std::vector<DVKPrimitive*> DVKPrimitives;

		DVKPrimitives		primitives;
		DVKBoundingBox		bounding;
        DVKNode*			linkNode;
        
		std::vector<int32>	bones;
        bool				isSkin = false;
        
		DVKMaterialInfo		material;
        
		int32				vertexCount;
		int32				triangleCount;
        
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
        
        Matrix4x4& GetGlobalMatrix()
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

			for (int32 i = 0; i < bones.size(); ++i) {
				delete bones[i];
			}
			bones.clear();
        }

		void Update(float time, float delta);

		void SetAnimation(int32 index);

		DVKAnimation& GetAnimation(int32 index = -1);

		void GotoAnimation(float time);

		VkVertexInputBindingDescription GetInputBinding();

		std::vector<VkVertexInputAttributeDescription> GetInputAttributes();
        
        static DVKModel* LoadFromFile(const std::string& filename, std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, const std::vector<VertexAttribute>& attributes);
        
        static DVKModel* Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, const std::vector<float>& vertices, const std::vector<uint16>& indices, const std::vector<VertexAttribute>& attributes);
        
    protected:
        
        DVKNode* LoadNode(const aiNode* node, const aiScene* scene);
        
		DVKMesh* LoadMesh(const aiMesh* mesh, const aiScene* scene);

		void LoadBones(const aiScene* aiScene);

        void LoadSkin(std::unordered_map<uint32, DVKVertexSkin>& skinInfoMap, DVKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene);

        void LoadVertexDatas(std::unordered_map<uint32, DVKVertexSkin>& skinInfoMap, std::vector<float>& vertices, Vector3& mmax, Vector3& mmin, DVKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene);
        
        void LoadIndices(std::vector<uint32>& indices, const aiMesh* aiMesh, const aiScene* aiScene);
        
        void LoadPrimitives(std::vector<float>& vertices, std::vector<uint32>& indices, DVKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene);
        
        void LoadAnim(const aiScene* aiScene);
        
    public:
        typedef std::unordered_map<std::string, DVKNode*> NodesMap;
		typedef std::unordered_map<std::string, DVKBone*> BonesMap;

        std::shared_ptr<VulkanDevice>	device;
        
        DVKNode*						rootNode;
        std::vector<DVKNode*>			linearNodes;
        std::vector<DVKMesh*>			meshes;

		NodesMap						nodesMap;

		std::vector<DVKBone*>			bones;
		BonesMap						bonesMap;
		
		std::vector<VertexAttribute>	attributes;
		std::vector<DVKAnimation>		animations;
		int32							animIndex = -1;

	private:

		DVKCommandBuffer*				cmdBuffer = nullptr;
        bool                            loadSkin = false;
    };
    
};
