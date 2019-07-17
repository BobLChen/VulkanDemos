#include "DVKModel.h"

#include "File/FileManager.h"

namespace vk_demo
{

	DVKModel* DVKModel::LoadFromFile(const std::string& filename, std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, const std::vector<VertexAttribute>& attributes)
    {
        DVKModel* model   = new DVKModel();
        model->device     = vulkanDevice;
		model->attributes = attributes;
		model->cmdBuffer  = cmdBuffer;
        
        int assimpFlags =
            aiProcess_Triangulate |
            aiProcess_MakeLeftHanded |
            aiProcess_FlipUVs |
            aiProcess_FlipWindingOrder;
        
        for (int32 i = 0; i < attributes.size(); ++i) {
            if (attributes[i] == VertexAttribute::VA_Tangent) {
                assimpFlags = assimpFlags | aiProcess_CalcTangentSpace;
            }
            else if (attributes[i] == VertexAttribute::VA_UV0) {
                assimpFlags = assimpFlags | aiProcess_GenUVCoords;
            }
            else if (attributes[i] == VertexAttribute::VA_Normal) {
                assimpFlags = assimpFlags | aiProcess_GenSmoothNormals;
            }
        }
        
        uint32 dataSize = 0;
        uint8* dataPtr  = nullptr;
        if (!FileManager::ReadFile(filename, dataPtr, dataSize)) {
            return model;
        }
        
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFileFromMemory(dataPtr, dataSize, assimpFlags);
        
		model->LoadNode(scene->mRootNode, scene);

        return model;
    }

	DVKMesh* DVKModel::LoadMesh(const aiMesh* aiMesh, const aiScene* aiScene)
	{
		DVKMesh* mesh = new DVKMesh();

		Vector3 mmin(MAX_flt, MAX_flt, MAX_flt);
		Vector3 mmax(MIN_flt, MIN_flt, MIN_flt);

		std::vector<float>  vertices;
		std::vector<uint32> indices;

		for (int32 i = 0; i < aiMesh->mNumVertices; ++i)
		{
			for (int32 j = 0; j < attributes.size(); ++j)
			{
				if (attributes[j] == VertexAttribute::VA_Position) 
				{
					float v0 = aiMesh->mVertices[i].x;
					float v1 = aiMesh->mVertices[i].y;
					float v2 = aiMesh->mVertices[i].z;

					vertices.push_back(v0);
					vertices.push_back(v1);
					vertices.push_back(v2);

					mmin.x = MMath::Min(v0, mmin.x);
					mmin.y = MMath::Min(v1, mmin.y);
					mmin.z = MMath::Min(v2, mmin.z);
					mmax.x = MMath::Max(v0, mmax.x);
					mmax.y = MMath::Max(v1, mmax.y);
					mmax.z = MMath::Max(v2, mmax.z);
				}
				else if (attributes[j] == VertexAttribute::VA_UV0) 
				{
					vertices.push_back(aiMesh->mTextureCoords[0][i].x);
					vertices.push_back(aiMesh->mTextureCoords[0][i].y);
				}
				else if (attributes[j] == VertexAttribute::VA_UV1) 
				{
					vertices.push_back(aiMesh->mTextureCoords[1][i].x);
					vertices.push_back(aiMesh->mTextureCoords[1][i].y);
				}
				else if (attributes[j] == VertexAttribute::VA_Normal) 
				{
					vertices.push_back(aiMesh->mNormals[i].x);
					vertices.push_back(aiMesh->mNormals[i].y);
					vertices.push_back(aiMesh->mNormals[i].z);
				}
				else if (attributes[j] == VertexAttribute::VA_Tangent) 
				{
					vertices.push_back(aiMesh->mTangents[i].x);
					vertices.push_back(aiMesh->mTangents[i].y);
					vertices.push_back(aiMesh->mTangents[i].z);
					vertices.push_back(1);
				}
				else if (attributes[j] == VertexAttribute::VA_Color) 
				{
					vertices.push_back(aiMesh->mColors[0][i].r);
					vertices.push_back(aiMesh->mColors[0][i].g);
					vertices.push_back(aiMesh->mColors[0][i].b);
				}
			}
		}

		for (int32 i = 0; i < aiMesh->mNumFaces; ++i)
		{
			indices.push_back(aiMesh->mFaces[i].mIndices[0]);
			indices.push_back(aiMesh->mFaces[i].mIndices[1]);
			indices.push_back(aiMesh->mFaces[i].mIndices[2]);
		}

		int32 stride = vertices.size() / aiMesh->mNumVertices;

		if (indices.size() > 65535) 
		{
			std::unordered_map<uint32, uint32> indicesMap;
			DVKPrimitive* primitive = nullptr;

			for (int32 i = 0; i < indices.size(); ++i) {
				uint32 idx = indices[i];
				if (primitive == nullptr) {
					primitive = new DVKPrimitive();
					indicesMap.clear();
					mesh->primitives.push_back(primitive);
				}

				uint32 newIdx = 0;
				auto it = indicesMap.find(idx);
				if (it == indicesMap.end()) 
				{
					uint32 start = idx * stride;
					newIdx = primitive->vertices.size() / stride;
					primitive->vertices.insert(primitive->vertices.end(), vertices.begin() + start, vertices.begin() + start + stride);
					indicesMap.insert(std::make_pair(idx, newIdx));
				}
				else
				{
					newIdx = it->second;
				}

				primitive->indices.push_back(newIdx);

				if (primitive->indices.size() == 65535) {
					primitive = nullptr;
				}
			}

			for (int32 i = 0; i < mesh->primitives.size(); ++i) 
			{
				primitive = mesh->primitives[i];
				primitive->vertexBuffer = DVKVertexBuffer::Create(device, cmdBuffer, primitive->vertices, attributes);
				primitive->indexBuffer  = DVKIndexBuffer::Create(device, cmdBuffer, primitive->indices);
			}
		}
		else
		{
			DVKPrimitive* primitive = new DVKPrimitive();
			primitive->vertices = vertices;
			for (uint16 i = 0; i < indices.size(); ++i) {
				primitive->indices.push_back(indices[i]);
			}
			primitive->vertexBuffer = DVKVertexBuffer::Create(device, cmdBuffer, primitive->vertices, attributes);
			primitive->indexBuffer  = DVKIndexBuffer::Create(device, cmdBuffer, primitive->indices);
			mesh->primitives.push_back(primitive);
		}

		for (int32 i = 0; i < mesh->primitives.size(); ++i)
		{
			mesh->vertexCount   += mesh->primitives[i]->vertices.size() / stride;
			mesh->triangleCount += mesh->primitives[i]->indices.size() / 3;
		}

		mesh->bounding.min = mmin;
		mesh->bounding.max = mmax;

		return mesh;
	}
    
    DVKNode* DVKModel::LoadNode(const aiNode* aiNode, const aiScene* aiScene)
    {
		DVKNode* vkNode = new DVKNode();
		vkNode->name = aiNode->mName.C_Str();

		if (rootNode == nullptr) {
			rootNode = vkNode;
		}

		// local matrix
		vkNode->localMatrix.m[0][0] = aiNode->mTransformation.a1;
		vkNode->localMatrix.m[0][1] = aiNode->mTransformation.a2;
		vkNode->localMatrix.m[0][2] = aiNode->mTransformation.a3;
		vkNode->localMatrix.m[0][3] = aiNode->mTransformation.a4;
		vkNode->localMatrix.m[1][0] = aiNode->mTransformation.b1;
		vkNode->localMatrix.m[1][1] = aiNode->mTransformation.b2;
		vkNode->localMatrix.m[1][2] = aiNode->mTransformation.b3;
		vkNode->localMatrix.m[1][3] = aiNode->mTransformation.b4;
		vkNode->localMatrix.m[2][0] = aiNode->mTransformation.c1;
		vkNode->localMatrix.m[2][1] = aiNode->mTransformation.c2;
		vkNode->localMatrix.m[2][2] = aiNode->mTransformation.c3;
		vkNode->localMatrix.m[2][3] = aiNode->mTransformation.c4;
		vkNode->localMatrix.m[3][0] = aiNode->mTransformation.d1;
		vkNode->localMatrix.m[3][1] = aiNode->mTransformation.d2;
		vkNode->localMatrix.m[3][2] = aiNode->mTransformation.d3;
		vkNode->localMatrix.m[3][3] = aiNode->mTransformation.d4;

		// mesh
        if (aiNode->mNumMeshes > 0) {
			for (int i = 0; i < aiNode->mNumMeshes; ++i) 
			{
				DVKMesh* vkMesh = LoadMesh(aiScene->mMeshes[aiNode->mMeshes[i]], aiScene);
				vkMesh->linkNode = vkNode;
				meshes.push_back(vkMesh);
				vkNode->meshes.push_back(vkMesh);
			}
        }
        
		linearNodes.push_back(vkNode);
		// children node
        for (int32 i = 0; i < aiNode->mNumChildren; ++i) 
		{
            DVKNode* childNode = LoadNode(aiNode->mChildren[i], aiScene);
			childNode->parent  = vkNode;
			vkNode->children.push_back(childNode);
        }

		vkNode->invalid = true;

		return vkNode;
    }
    
	VkVertexInputBindingDescription DVKModel::GetInputBinding()
	{
		int32 stride = 0;
		for (int32 i = 0; i < attributes.size(); ++i) {
			stride += VertexAttributeToSize(attributes[i]);
		}

		VkVertexInputBindingDescription vertexInputBinding = {};
		vertexInputBinding.binding   = 0;
		vertexInputBinding.stride    = stride;
		vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vertexInputBinding;
	}

	std::vector<VkVertexInputAttributeDescription> DVKModel::GetInputAttributes()
	{
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
		int32 offset = 0;

		for (int32 i = 0; i < attributes.size(); ++i)
		{
			VkVertexInputAttributeDescription inputAttribute = {};
			inputAttribute.binding  = 0;
			inputAttribute.location = i;
			inputAttribute.format   = VertexAttributeToVkFormat(attributes[i]);
			inputAttribute.offset   = offset;
			offset += VertexAttributeToSize(attributes[i]);
			vertexInputAttributs.push_back(inputAttribute);
		}

		return vertexInputAttributs;
	}

}
