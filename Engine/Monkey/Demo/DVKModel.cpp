#include "DVKModel.h"

#include "File/FileManager.h"
#include "Math/Matrix4x4.h"

#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace vk_demo
{
	void SimplifyTexturePath(std::string& path)
	{
		const size_t lastSlashIdx = path.find_last_of("\\/");
		if (std::string::npos != lastSlashIdx) {
			path.erase(0, lastSlashIdx + 1);
		}

		const size_t periodIdx = path.rfind('.');
		if (std::string::npos != periodIdx) {
			path.erase(periodIdx);
		}
	}

	void FillMaterialTextures(aiMaterial* aiMaterial, DVKMaterialInfo& material)
	{
		if (aiMaterial->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE)) {
			aiString texturePath;
			aiMaterial->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &texturePath);
			material.diffuse = texturePath.C_Str();
			SimplifyTexturePath(material.diffuse);
		}
		
		if (aiMaterial->GetTextureCount(aiTextureType::aiTextureType_NORMALS)) {
			aiString texturePath;
			aiMaterial->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &texturePath);
			material.normalmap = texturePath.C_Str();
			SimplifyTexturePath(material.normalmap);
		}

		if (aiMaterial->GetTextureCount(aiTextureType::aiTextureType_SPECULAR)) {
			aiString texturePath;
			aiMaterial->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &texturePath);
			material.specular = texturePath.C_Str();
			SimplifyTexturePath(material.specular);
		}
	}
    
    void FillMatrixWithAiMatrix(Matrix4x4& matrix, const aiMatrix4x4& aiMatrix)
    {
        matrix.m[0][0] = aiMatrix.a1;
        matrix.m[0][1] = aiMatrix.a2;
        matrix.m[0][2] = aiMatrix.a3;
        matrix.m[0][3] = aiMatrix.a4;
        matrix.m[1][0] = aiMatrix.b1;
        matrix.m[1][1] = aiMatrix.b2;
        matrix.m[1][2] = aiMatrix.b3;
        matrix.m[1][3] = aiMatrix.b4;
        matrix.m[2][0] = aiMatrix.c1;
        matrix.m[2][1] = aiMatrix.c2;
        matrix.m[2][2] = aiMatrix.c3;
        matrix.m[2][3] = aiMatrix.c4;
        matrix.m[3][0] = aiMatrix.d1;
        matrix.m[3][1] = aiMatrix.d2;
        matrix.m[3][2] = aiMatrix.d3;
        matrix.m[3][3] = aiMatrix.d4;
        matrix.SetTransposed();
    }
    
    DVKModel* DVKModel::Create(std::shared_ptr<VulkanDevice> vulkanDevice, DVKCommandBuffer* cmdBuffer, const std::vector<float>& vertices, const std::vector<uint16>& indices, const std::vector<VertexAttribute>& attributes)
    {
        DVKModel* model   = new DVKModel();
        model->device     = vulkanDevice;
        model->attributes = attributes;
        model->cmdBuffer  = cmdBuffer;
        
        DVKPrimitive* primitive = new DVKPrimitive();
        primitive->vertices = vertices;
        primitive->indices  = indices;
        
        if (cmdBuffer)
        {
            primitive->vertexBuffer = DVKVertexBuffer::Create(vulkanDevice, cmdBuffer, primitive->vertices, attributes);
            primitive->indexBuffer  = DVKIndexBuffer::Create(vulkanDevice, cmdBuffer, primitive->indices);
        }
        
        DVKMesh* mesh = new DVKMesh();
        mesh->primitives.push_back(primitive);
        mesh->bounding.min = Vector3(-1.0f, -1.0f, 0.0f);
        mesh->bounding.max = Vector3(1.0f, 1.0f, 0.0f);
        
        DVKNode* rootNode = new DVKNode();
        rootNode->name = "RootNode";
        rootNode->meshes.push_back(mesh);
        rootNode->localMatrix.SetIdentity();
        mesh->linkNode = rootNode;
        
        model->rootNode = rootNode;
        model->meshes.push_back(mesh);
        
        return model;
    }

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
            else if (attributes[i] == VertexAttribute::VA_SkinIndex) {
                model->loadSkin = true;
            }
            else if (attributes[i] == VertexAttribute::VA_SkinWeight) {
                model->loadSkin = true;
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
        model->LoadAnim(scene);
        
        return model;
    }
    
    void DVKModel::LoadSkin(std::unordered_map<uint32, VertexSkin>& skinInfoMap, DVKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene)
    {
        std::unordered_map<std::string, int32> boneIndexMap;
        
        for (int32 i = 0; i < aiMesh->mNumBones; ++i)
        {
            aiBone* boneInfo = aiMesh->mBones[i];
            std::string boneName(boneInfo->mName.C_Str());
            int32 bondIndex  = 0;
            // 收集Bone信息并编号
            auto it = boneIndexMap.find(boneName);
            if (it == boneIndexMap.end())
            {
                bondIndex    = mesh->bones.size();
                DVKBone bone = {};
                bone.index   = bondIndex;
                bone.parent  = -1;
                bone.name    = boneName;
                FillMatrixWithAiMatrix(bone.inverseBindPose, boneInfo->mOffsetMatrix);
                mesh->bones.push_back(bone);
                boneIndexMap.insert(std::make_pair(boneName, bondIndex));
            }
            else
            {
                bondIndex = it->second;
            }
            // 收集被Bone影响的顶点信息
            for (uint32 j = 0; j < boneInfo->mNumWeights; ++j)
            {
                uint32 vertexID  = boneInfo->mWeights[j].mVertexId;
                float  weight    = boneInfo->mWeights[j].mWeight;
                VertexSkin* info = nullptr;
                // 顶点->Bone
                if (skinInfoMap.find(vertexID) == skinInfoMap.end()) {
                    skinInfoMap.insert(std::make_pair(vertexID, VertexSkin()));
                }
                info = &(skinInfoMap[vertexID]);
                // 只允许最多四个骨骼影响顶点
                if (info->used >= 4) {
                    break;
                }
                info->indices[info->used] = bondIndex;
                info->weights[info->used] = weight;
                info->used += 1;
            }
        }
        // 再次处理一遍skinInfoMap，把未使用的补齐
        for (auto it = skinInfoMap.begin(); it != skinInfoMap.end(); ++it)
        {
            VertexSkin& info = it->second;
            for (int32 i = info.used; i < 4; ++i)
            {
                info.indices[i] = 0;
                info.weights[i] = 0.0f;
            }
        }
        
        mesh->isSkin = true;
    }
    
    void DVKModel::LoadVertexDatas(std::unordered_map<uint32, VertexSkin>& skinInfoMap, std::vector<float>& vertices, Vector3& mmax, Vector3& mmin, DVKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene)
    {
        Vector3 defaultColor = Vector3(MMath::RandRange(0.0f, 1.0f), MMath::RandRange(0.0f, 1.0f), MMath::RandRange(0.0f, 1.0f));
        
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
                    if (aiMesh->HasVertexColors(i))
                    {
                        vertices.push_back(aiMesh->mColors[0][i].r);
                        vertices.push_back(aiMesh->mColors[0][i].g);
                        vertices.push_back(aiMesh->mColors[0][i].b);
                    }
                    else
                    {
                        vertices.push_back(defaultColor.x);
                        vertices.push_back(defaultColor.y);
                        vertices.push_back(defaultColor.z);
                    }
                }
                else if (attributes[j] == VertexAttribute::VA_SkinIndex)
                {
                    if (mesh->isSkin)
                    {
                        VertexSkin& skin = skinInfoMap[i];
                        vertices.push_back(skin.indices[0]);
                        vertices.push_back(skin.indices[1]);
                        vertices.push_back(skin.indices[2]);
                        vertices.push_back(skin.indices[3]);
                    }
                    else
                    {
                        vertices.push_back(0);
                        vertices.push_back(0);
                        vertices.push_back(0);
                        vertices.push_back(0);
                    }
                }
                else if (attributes[j] == VertexAttribute::VA_SkinWeight)
                {
                    if (mesh->isSkin)
                    {
                        VertexSkin& skin = skinInfoMap[i];
                        vertices.push_back(skin.weights[0]);
                        vertices.push_back(skin.weights[1]);
                        vertices.push_back(skin.weights[2]);
                        vertices.push_back(skin.weights[3]);
                    }
                    else
                    {
                        vertices.push_back(0);
                        vertices.push_back(0);
                        vertices.push_back(0);
                        vertices.push_back(0);
                    }
                }
                else if (attributes[j] == VertexAttribute::VA_Custom0 ||
                         attributes[j] == VertexAttribute::VA_Custom1 ||
                         attributes[j] == VertexAttribute::VA_Custom2 ||
                         attributes[j] == VertexAttribute::VA_Custom3
                )
                {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }
            }
        }
    }
    
    void DVKModel::LoadIndices(std::vector<uint32>& indices, const aiMesh* aiMesh, const aiScene* aiScene)
    {
        for (int32 i = 0; i < aiMesh->mNumFaces; ++i)
        {
            indices.push_back(aiMesh->mFaces[i].mIndices[0]);
            indices.push_back(aiMesh->mFaces[i].mIndices[1]);
            indices.push_back(aiMesh->mFaces[i].mIndices[2]);
        }
    }
    
    void DVKModel::LoadPrimitives(std::vector<float>& vertices, std::vector<uint32>& indices, DVKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene)
    {
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
            
            if (cmdBuffer)
            {
                for (int32 i = 0; i < mesh->primitives.size(); ++i)
                {
                    primitive = mesh->primitives[i];
                    primitive->vertexBuffer = DVKVertexBuffer::Create(device, cmdBuffer, primitive->vertices, attributes);
                    primitive->indexBuffer  = DVKIndexBuffer::Create(device, cmdBuffer, primitive->indices);
                }
            }
        }
        else
        {
            DVKPrimitive* primitive = new DVKPrimitive();
            primitive->vertices = vertices;
            for (uint16 i = 0; i < indices.size(); ++i) {
                primitive->indices.push_back(indices[i]);
            }
            mesh->primitives.push_back(primitive);
            
            if (cmdBuffer)
            {
                primitive->vertexBuffer = DVKVertexBuffer::Create(device, cmdBuffer, primitive->vertices, attributes);
                primitive->indexBuffer  = DVKIndexBuffer::Create(device, cmdBuffer, primitive->indices);
            }
        }
        
        for (int32 i = 0; i < mesh->primitives.size(); ++i)
        {
            DVKPrimitive* primitive = mesh->primitives[i];
            primitive->vertexCount  = primitive->vertices.size() / stride;
            primitive->indexCount   = primitive->indices.size() / 3;
            
            mesh->vertexCount   += primitive->vertexCount;
            mesh->triangleCount += primitive->indexCount;
        }
    }
    
	DVKMesh* DVKModel::LoadMesh(const aiMesh* aiMesh, const aiScene* aiScene)
	{
		DVKMesh* mesh = new DVKMesh();

        // load material
		aiMaterial* material = aiScene->mMaterials[aiMesh->mMaterialIndex];
		if (material) {
			FillMaterialTextures(material, mesh->material);
		}
        
        // load bones
        std::unordered_map<uint32, VertexSkin> skinInfoMap;
        if (aiMesh->mNumBones > 0 && loadSkin) {
            LoadSkin(skinInfoMap, mesh, aiMesh, aiScene);
        }
        
        // load vertex data
        std::vector<float> vertices;
        Vector3 mmin(MAX_flt, MAX_flt, MAX_flt);
        Vector3 mmax(MIN_flt, MIN_flt, MIN_flt);
        LoadVertexDatas(skinInfoMap, vertices, mmax, mmin, mesh, aiMesh, aiScene);
        
        // load indices
        std::vector<uint32> indices;
        LoadIndices(indices, aiMesh, aiScene);

		// load primitives
        LoadPrimitives(vertices, indices, mesh, aiMesh, aiScene);
        
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
        FillMatrixWithAiMatrix(vkNode->localMatrix, aiNode->mTransformation);
        
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
        
		return vkNode;
    }
    
    void DVKModel::LoadAnim(const aiScene* aiScene)
    {
        for (int32 i = 0; i < aiScene->mNumAnimations; ++i)
        {
            aiAnimation* animation = aiScene->mAnimations[i];
            for (int32 j = 0; j < animation->mNumChannels; ++j)
            {
                aiNodeAnim* anim = animation->mChannels[j];
            }
        }
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
