#include "DVKModel.h"

#include "File/FileManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace vk_demo
{
    DVKModel* DVKModel::LoadFromFile(const std::string& filename, std::shared_ptr<VulkanDevice> vulkanDevice, std::vector<VertexAttribute> attributes)
    {
        VkDevice device = vulkanDevice->GetInstanceHandle();
        
        DVKModel* model = new DVKModel();
        model->device = device;
        
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
        
        model->LoadNode(scene->mRootNode);
        
        return model;
    }
    
    void DVKModel::LoadNode(aiNode* node)
    {
        std::string name = node->mName.C_Str();
        printf("name=%s\n", name.c_str());
        
        if (node->mNumMeshes > 0) {
            for (int32 i = 0; i < node->mNumMeshes; ++i) {
                printf("\tLinkMesh=%d\n", node->mMeshes[i]);
            }
        }
        
        for (int32 i = 0; i < node->mNumChildren; ++i) {
            LoadNode(node->mChildren[i]);
        }
    }
    
}
