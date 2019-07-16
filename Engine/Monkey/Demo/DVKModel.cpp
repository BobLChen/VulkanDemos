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
		
		for (int32 i = 0; i < aiMesh->mNumVertices; ++i)
		{
			for (int32 j = 0; j < attributes.size(); ++j)
			{
				if (attributes[j] == VertexAttribute::VA_Position) {
					float v0 = aiMesh->mVertices[i].x;
					float v1 = aiMesh->mVertices[i].y;
					float v2 = aiMesh->mVertices[i].z;

					mesh->vertices.push_back(v0);
					mesh->vertices.push_back(v1);
					mesh->vertices.push_back(v2);

					mmin.x = MMath::Min(v0, mmin.x);
					mmin.y = MMath::Min(v1, mmin.y);
					mmin.z = MMath::Min(v2, mmin.z);
					mmax.x = MMath::Max(v0, mmax.x);
					mmax.y = MMath::Max(v1, mmax.y);
					mmax.z = MMath::Max(v2, mmax.z);
				}
				else if (attributes[j] == VertexAttribute::VA_UV0) {
					mesh->vertices.push_back(aiMesh->mTextureCoords[0][i].x);
					mesh->vertices.push_back(aiMesh->mTextureCoords[0][i].y);
				}
				else if (attributes[j] == VertexAttribute::VA_UV1) {
					mesh->vertices.push_back(aiMesh->mTextureCoords[1][i].x);
					mesh->vertices.push_back(aiMesh->mTextureCoords[1][i].y);
				}
				else if (attributes[j] == VertexAttribute::VA_Normal) {
					mesh->vertices.push_back(aiMesh->mNormals[i].x);
					mesh->vertices.push_back(aiMesh->mNormals[i].y);
					mesh->vertices.push_back(aiMesh->mNormals[i].z);
				}
				else if (attributes[j] == VertexAttribute::VA_Tangent) {
					mesh->vertices.push_back(aiMesh->mTangents[i].x);
					mesh->vertices.push_back(aiMesh->mTangents[i].y);
					mesh->vertices.push_back(aiMesh->mTangents[i].z);
				}
				else if (attributes[j] == VertexAttribute::VA_Color) {
					mesh->vertices.push_back(aiMesh->mColors[0][i].r);
					mesh->vertices.push_back(aiMesh->mColors[0][i].g);
					mesh->vertices.push_back(aiMesh->mColors[0][i].b);
				}
			}
		}

		for (int32 i = 0; i < aiMesh->mNumFaces; ++i)
		{
			mesh->indices.push_back(aiMesh->mFaces[i].mIndices[0]);
			mesh->indices.push_back(aiMesh->mFaces[i].mIndices[1]);
			mesh->indices.push_back(aiMesh->mFaces[i].mIndices[2]);
		}

		mesh->bounding.min = mmin;
		mesh->bounding.max = mmax;

		mesh->vertexBuffer = DVKVertexBuffer::Create(device, cmdBuffer, mesh->vertices, attributes);
		mesh->indexBuffer  = DVKIndexBuffer::Create(device, cmdBuffer, mesh->indices);

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
			DVKMesh* vkMesh  = LoadMesh(aiScene->mMeshes[aiNode->mMeshes[0]], aiScene);
			vkNode->mesh     = vkMesh;
			vkMesh->linkNode = vkNode;
			meshes.push_back(vkMesh);
        }
        
		linearNodes.push_back(vkNode);
		// children node
        for (int32 i = 0; i < aiNode->mNumChildren; ++i) {
            DVKNode* childNode = LoadNode(aiNode->mChildren[i], aiScene);
			childNode->parent  = vkNode;
			vkNode->children.push_back(childNode);
        }

		vkNode->invalid = true;

		return vkNode;
    }
    
	VkVertexInputBindingDescription DVKModel::GetInputBinding()
	{
		return meshes[0]->vertexBuffer->GetInputBinding();
	}

	std::vector<VkVertexInputAttributeDescription> DVKModel::GetInputAttributes(const std::vector<VertexAttribute>& shaderInputs)
	{
		return meshes[0]->vertexBuffer->GetInputAttributes(shaderInputs);
	}

}
