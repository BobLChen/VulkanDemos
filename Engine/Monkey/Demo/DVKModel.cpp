#include "DVKModel.h"

#include "FileManager.h"
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
		if (aiMaterial->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE)) 
		{
			aiString texturePath;
			aiMaterial->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &texturePath);
			material.diffuse = texturePath.C_Str();
			SimplifyTexturePath(material.diffuse);
		}
		
		if (aiMaterial->GetTextureCount(aiTextureType::aiTextureType_NORMALS)) 
		{
			aiString texturePath;
			aiMaterial->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &texturePath);
			material.normalmap = texturePath.C_Str();
			SimplifyTexturePath(material.normalmap);
		}

		if (aiMaterial->GetTextureCount(aiTextureType::aiTextureType_SPECULAR)) 
		{
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
        
		int32 stride = 0;
		for (int32 i = 0; i < attributes.size(); ++i) {
			stride += VertexAttributeToSize(attributes[i]);
		}

        DVKPrimitive* primitive = new DVKPrimitive();
        primitive->vertices = vertices;
        primitive->indices  = indices;
		primitive->vertexCount = vertices.size() / stride * 4;
		
        if (cmdBuffer)
        {
			if (vertices.size() > 0) {
				primitive->vertexBuffer = DVKVertexBuffer::Create(vulkanDevice, cmdBuffer, primitive->vertices, attributes);
			}
			if (indices.size() > 0) {
				primitive->indexBuffer = DVKIndexBuffer::Create(vulkanDevice, cmdBuffer, primitive->indices);
			}
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
        
        int assimpFlags = aiProcess_Triangulate | aiProcess_FlipUVs;
		
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
			else if (attributes[i] == VertexAttribute::VA_SkinPack) {
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
        
		model->LoadBones(scene);
		model->LoadNode(scene->mRootNode, scene);
        model->LoadAnim(scene);

		delete[] dataPtr;
        
        return model;
    }

	void DVKModel::LoadBones(const aiScene* aiScene)
	{
		std::unordered_map<std::string, int32> boneIndexMap;
		for (int32 i = 0; i < aiScene->mNumMeshes; ++i)
		{
			aiMesh* aimesh = aiScene->mMeshes[i];
			for (int32 j = 0; j < aimesh->mNumBones; ++j)
			{
				aiBone* aibone   = aimesh->mBones[j];
				std::string name = aibone->mName.C_Str();

				auto it = boneIndexMap.find(name);
				if (it == boneIndexMap.end())
				{
					// new bone
					int32 index   = bones.size();
					DVKBone* bone = new DVKBone();
					bone->index   = index;
					bone->parent  = -1;
					bone->name    = name;
					FillMatrixWithAiMatrix(bone->inverseBindPose, aibone->mOffsetMatrix);
					// 记录Bone信息
					bones.push_back(bone);
					bonesMap.insert(std::make_pair(name, bone));
					// cache
					boneIndexMap.insert(std::make_pair(name, index));
				}
			}
		}
	}

    void DVKModel::LoadSkin(std::unordered_map<uint32, DVKVertexSkin>& skinInfoMap, DVKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene)
    {
        std::unordered_map<int32, int32> boneIndexMap;
        
        for (int32 i = 0; i < aiMesh->mNumBones; ++i)
        {
            aiBone* boneInfo = aiMesh->mBones[i];
            std::string boneName(boneInfo->mName.C_Str());
			int32 boneIndex = bonesMap[boneName]->index;

			// bone在mesh中的索引
			int32 meshBoneIndex = 0;
			auto it = boneIndexMap.find(boneIndex);
			if (it == boneIndexMap.end()) {
				meshBoneIndex = mesh->bones.size();
				mesh->bones.push_back(boneIndex);
				boneIndexMap.insert(std::make_pair(boneIndex, meshBoneIndex));
			}
			else
			{
				meshBoneIndex = it->second;
			}

            // 收集被Bone影响的顶点信息
            for (uint32 j = 0; j < boneInfo->mNumWeights; ++j)
            {
                uint32 vertexID = boneInfo->mWeights[j].mVertexId;
                float  weight   = boneInfo->mWeights[j].mWeight;
                // 顶点->Bone
                if (skinInfoMap.find(vertexID) == skinInfoMap.end()) {
                    skinInfoMap.insert(std::make_pair(vertexID, DVKVertexSkin()));
                }
				DVKVertexSkin* info = &(skinInfoMap[vertexID]);
                info->indices[info->used] = meshBoneIndex;
                info->weights[info->used] = weight;
                info->used += 1;
				// 只允许最多四个骨骼影响顶点
				if (info->used >= 4) {
					break;
				}
            }
        }

        // 再次处理一遍skinInfoMap，把未使用的补齐
        for (auto it = skinInfoMap.begin(); it != skinInfoMap.end(); ++it)
        {
			DVKVertexSkin& info = it->second;
            for (int32 i = info.used; i < 4; ++i)
            {
                info.indices[i] = 0;
                info.weights[i] = 0.0f;
            }
        }
        
        mesh->isSkin = true;
    }
    
    void DVKModel::LoadVertexDatas(std::unordered_map<uint32, DVKVertexSkin>& skinInfoMap, std::vector<float>& vertices, Vector3& mmax, Vector3& mmin, DVKMesh* mesh, const aiMesh* aiMesh, const aiScene* aiScene)
    {
        Vector3 defaultColor(
			MMath::RandRange(0.0f, 1.0f), 
			MMath::RandRange(0.0f, 1.0f), 
			MMath::RandRange(0.0f, 1.0f)
		);
        
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
					if (aiMesh->HasTextureCoords(0)) 
					{
						vertices.push_back(aiMesh->mTextureCoords[0][i].x);
						vertices.push_back(aiMesh->mTextureCoords[0][i].y);
					}
					else 
					{
						vertices.push_back(0);
						vertices.push_back(0);
					}
                }
                else if (attributes[j] == VertexAttribute::VA_UV1)
                {
					if (aiMesh->HasTextureCoords(1))
					{
						vertices.push_back(aiMesh->mTextureCoords[1][i].x);
						vertices.push_back(aiMesh->mTextureCoords[1][i].y);
					}
					else
					{
						vertices.push_back(0);
						vertices.push_back(0);
					}
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
				else if (attributes[j] == VertexAttribute::VA_SkinPack)
				{
					if (mesh->isSkin)
					{
						DVKVertexSkin& skin = skinInfoMap[i];

						int32 idx0 = skin.indices[0];
						int32 idx1 = skin.indices[1];
						int32 idx2 = skin.indices[2];
						int32 idx3 = skin.indices[3];
						uint32 packIndex = (idx0 << 24) + (idx1 << 16) + (idx2 << 8) + idx3;

						uint16 weight0 = skin.weights[0] * 65535;
						uint16 weight1 = skin.weights[1] * 65535;
						uint16 weight2 = skin.weights[2] * 65535;
						uint16 weight3 = skin.weights[3] * 65535;
						uint32 packWeight0 = (weight0 << 16) + weight1;
						uint32 packWeight1 = (weight2 << 16) + weight3;

						vertices.push_back(packIndex);
						vertices.push_back(packWeight0);
						vertices.push_back(packWeight1);
					}
					else
					{
						vertices.push_back(0);
						vertices.push_back(65535);
						vertices.push_back(0);
					}
				}
                else if (attributes[j] == VertexAttribute::VA_SkinIndex)
                {
                    if (mesh->isSkin)
                    {
						DVKVertexSkin& skin = skinInfoMap[i];
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
						DVKVertexSkin& skin = skinInfoMap[i];
                        vertices.push_back(skin.weights[0]);
                        vertices.push_back(skin.weights[1]);
                        vertices.push_back(skin.weights[2]);
                        vertices.push_back(skin.weights[3]);
                    }
                    else
                    {
                        vertices.push_back(1.0f);
                        vertices.push_back(0.0f);
                        vertices.push_back(0.0f);
                        vertices.push_back(0.0f);
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
                if (primitive == nullptr) 
				{
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
            primitive->triangleNum  = primitive->indices.size() / 3;
            
            mesh->vertexCount   += primitive->vertexCount;
            mesh->triangleCount += primitive->triangleNum;
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
        std::unordered_map<uint32, DVKVertexSkin> skinInfoMap;
        if (aiMesh->mNumBones > 0 && loadSkin) {
            LoadSkin(skinInfoMap, mesh, aiMesh, aiScene);
        }
        
        // load vertex data
        std::vector<float> vertices;
        Vector3 mmin( MAX_int32,  MAX_int32,  MAX_int32);
        Vector3 mmax(-MAX_int32, -MAX_int32, -MAX_int32);
        LoadVertexDatas(skinInfoMap, vertices, mmax, mmin, mesh, aiMesh, aiScene);
        
        // load indices
        std::vector<uint32> indices;
        LoadIndices(indices, aiMesh, aiScene);

		// load primitives
        LoadPrimitives(vertices, indices, mesh, aiMesh, aiScene);
        
		mesh->bounding.min = mmin;
		mesh->bounding.max = mmax;
		mesh->bounding.UpdateCorners();

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
				DVKMesh* vkMesh  = LoadMesh(aiScene->mMeshes[aiNode->mMeshes[i]], aiScene);
				vkMesh->linkNode = vkNode;
				vkNode->meshes.push_back(vkMesh);
				meshes.push_back(vkMesh);
			}
        }
        
		// nodes map
		nodesMap.insert(std::make_pair(vkNode->name, vkNode));
		linearNodes.push_back(vkNode);

		// bones parent
		int32 boneParentIndex = -1;
		{
			auto it = bonesMap.find(vkNode->name);
			if (it != bonesMap.end()) {
				boneParentIndex = it->second->index;
			}
		}
		
		// children node
        for (int32 i = 0; i < aiNode->mNumChildren; ++i) 
		{
            DVKNode* childNode = LoadNode(aiNode->mChildren[i], aiScene);
			childNode->parent  = vkNode;
			vkNode->children.push_back(childNode);

			// bones relationship
			{
				auto it = bonesMap.find(childNode->name);
				if (it != bonesMap.end()) {
					it->second->parent = boneParentIndex;
				}
			}
        }
        
		return vkNode;
    }
    
    void DVKModel::LoadAnim(const aiScene* aiScene)
    {
        for (int32 i = 0; i < aiScene->mNumAnimations; ++i)
        {
            aiAnimation* aianimation = aiScene->mAnimations[i];
			float timeTick = aianimation->mTicksPerSecond != 0 ? aianimation->mTicksPerSecond : 25.0f;

			animations.push_back(DVKAnimation());
			DVKAnimation& dvkAnimation = animations.back();
            
			for (int32 j = 0; j < aianimation->mNumChannels; ++j)
            {
                aiNodeAnim* nodeAnim = aianimation->mChannels[j];
				std::string nodeName = nodeAnim->mNodeName.C_Str();

				dvkAnimation.clips.insert(std::make_pair(nodeName, DVKAnimationClip()));

				DVKAnimationClip& animClip = dvkAnimation.clips[nodeName];
				animClip.nodeName = nodeName;
				animClip.duration = 0.0f;
				
				// position
				for (int32 index = 0; index < nodeAnim->mNumPositionKeys; ++index)
				{
					aiVectorKey& aikey = nodeAnim->mPositionKeys[index];
					animClip.positions.keys.push_back(aikey.mTime / timeTick);
					animClip.positions.values.push_back(Vector3(aikey.mValue.x, aikey.mValue.y, aikey.mValue.z));
					animClip.duration = MMath::Max((float)aikey.mTime / timeTick, animClip.duration);
				}

				// scale
				for (int32 index = 0; index < nodeAnim->mNumScalingKeys; ++index)
				{
					aiVectorKey& aikey = nodeAnim->mScalingKeys[index];
					animClip.scales.keys.push_back(aikey.mTime / timeTick);
					animClip.scales.values.push_back(Vector3(aikey.mValue.x, aikey.mValue.y, aikey.mValue.z));
					animClip.duration = MMath::Max((float)aikey.mTime / timeTick, animClip.duration);
				}

				// rotation
				for (int32 index = 0; index < nodeAnim->mNumRotationKeys; ++index)
				{
					aiQuatKey& aikey = nodeAnim->mRotationKeys[index];
					animClip.rotations.keys.push_back(aikey.mTime / timeTick);
					animClip.rotations.values.push_back(Quat(aikey.mValue.x, aikey.mValue.y, aikey.mValue.z, aikey.mValue.w));
					animClip.duration = MMath::Max((float)aikey.mTime / timeTick, animClip.duration);
				}

				dvkAnimation.duration = MMath::Max(animClip.duration, dvkAnimation.duration);
            }
        }
    }

	void DVKModel::GotoAnimation(float time)
	{
		if (animIndex == -1) {
			return;
		}
        
		DVKAnimation& animation = animations[animIndex];
        animation.time = MMath::Clamp(time, 0.0f, animation.duration);
        
		// update nodes animation
		for (auto it = animation.clips.begin(); it != animation.clips.end(); ++it)
		{
			vk_demo::DVKAnimationClip& clip = it->second;
			vk_demo::DVKNode* node = nodesMap[clip.nodeName];
            
			float alpha = 0.0f;
            
			// rotation
			Quat prevRot(0, 0, 0, 1);
			Quat nextRot(0, 0, 0, 1);
			clip.rotations.GetValue(animation.time, prevRot, nextRot, alpha);
			Quat retRot = MMath::Lerp(prevRot, nextRot, alpha);
            
			// position
			Vector3 prevPos(0, 0, 0);
			Vector3 nextPos(0, 0, 0);
			clip.positions.GetValue(animation.time, prevPos, nextPos, alpha);
			Vector3 retPos = MMath::Lerp(prevPos, nextPos, alpha);
            
			// scale
			Vector3 prevScale(1, 1, 1);
			Vector3 nextScale(1, 1, 1);
			clip.scales.GetValue(animation.time, prevScale, nextScale, alpha);
			Vector3 retScale = MMath::Lerp(prevScale, nextScale, alpha);
            
			node->localMatrix.SetIdentity();
			node->localMatrix.AppendScale(retScale);
			node->localMatrix.Append(retRot.ToMatrix());
			node->localMatrix.AppendTranslation(retPos);
		}

		// update bones
		for (int32 i = 0; i < bones.size(); ++i)
		{
			DVKBone* bone = bones[i];
			DVKNode* node = nodesMap[bone->name];
			// 注意行列矩阵的区别
			bone->finalTransform = bone->inverseBindPose;
			bone->finalTransform.Append(node->GetGlobalMatrix());
		}
	}
    
	void DVKModel::Update(float time, float delta)
	{
		if (animIndex == -1) {
			return;
		}
        
		DVKAnimation& animation = animations[animIndex];
		animation.time += delta * animation.speed;

		if (animation.time >= animation.duration) {
			animation.time = animation.time - animation.duration;
		}
        
		GotoAnimation(animation.time);
	}

	void DVKModel::SetAnimation(int32 index)
	{
		if (index >= animations.size()) {
			return;
		}
		if (index < 0) {
			return;
		}
		animIndex = index;
	}

	DVKAnimation& DVKModel::GetAnimation(int32 index)
	{
		if (index == -1) {
			index = animIndex;
		}
		return animations[index];
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
