#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

#include "File/FileManager.h"
#include "MeshLoader.h"

std::vector<std::shared_ptr<Renderable>> MeshLoader::LoadFromFile(const std::string& filename)
{

	std::vector<std::shared_ptr<Renderable>> renderables;

	static const int assimpFlags = aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_PreTransformVertices;
	
	const aiScene* scene = nullptr;
	Assimp::Importer importer;

	scene = importer.ReadFile(FileManager::GetFilePath(filename).c_str(), assimpFlags);

	for (uint32 i = 0; i < scene->mNumMeshes; ++i)
	{
		aiMesh* mesh = scene->mMeshes[i];

		std::vector<float> vertices;
		std::vector<uint32> indices;
		uint32 channelMask = 0;

		for (uint32 j = 0; j < mesh->mNumVertices; ++j)
		{
			if (mesh->HasPositions())
			{
				channelMask = channelMask | (1 << VertexAttribute::VA_Position);
				vertices.push_back(mesh->mVertices[j].x);
				vertices.push_back(mesh->mVertices[j].y);
				vertices.push_back(mesh->mVertices[j].z);
			}
			if (mesh->HasTextureCoords(0))
			{
				channelMask = channelMask | (1 << VertexAttribute::VA_UV0);
				vertices.push_back(mesh->mTextureCoords[0][j].x);
				vertices.push_back(mesh->mTextureCoords[0][j].y);
			}
			if (mesh->HasTextureCoords(1))
			{
				channelMask = channelMask | (1 << VertexAttribute::VA_UV1);
				vertices.push_back(mesh->mTextureCoords[1][j].x);
				vertices.push_back(mesh->mTextureCoords[1][j].y);
			}
			if (mesh->HasVertexColors(0))
			{
				channelMask = channelMask | (1 << VertexAttribute::VA_Color);
				vertices.push_back(mesh->mColors[0][j].r);
				vertices.push_back(mesh->mColors[0][j].g);
				vertices.push_back(mesh->mColors[0][j].b);
				vertices.push_back(mesh->mColors[0][j].a);
			}
			if (mesh->HasNormals()) 
			{
				channelMask = channelMask | (1 << VertexAttribute::VA_Normal);
				vertices.push_back(mesh->mNormals[j].x);
				vertices.push_back(mesh->mNormals[j].y);
				vertices.push_back(mesh->mNormals[j].z);
			}
			if (mesh->HasTangentsAndBitangents())
			{
				channelMask = channelMask | (1 << VertexAttribute::VA_Tangent);
				vertices.push_back(mesh->mTangents[j].x);
				vertices.push_back(mesh->mTangents[j].y);
				vertices.push_back(mesh->mTangents[j].z);
			}
		}

		for (uint32 j = 0; j < mesh->mNumFaces; ++j)
		{
			aiFace face = mesh->mFaces[j];
			for (uint32 m = 0; m < face.mNumIndices; ++m)
			{
				indices.push_back(face.mIndices[m]);
			}
		}
		
		std::vector<VertexChannelInfo> channels;
		int32 offset = 0;
		// position
		if (channelMask & (1 << VertexAttribute::VA_Position))
		{
			VertexChannelInfo channel = {};
			channel.attribute = VertexAttribute::VA_Position;
			channel.format = VertexElementType::VET_Float3;
			channel.stream = 0;
			channel.offset = offset;
			channels.push_back(channel);
			offset += 12;
		}
		// uv
		if (channelMask & (1 << VertexAttribute::VA_UV0))
		{
			VertexChannelInfo channel = {};
			channel.attribute = VertexAttribute::VA_UV0;
			channel.format = VertexElementType::VET_Float2;
			channel.stream = 0;
			channel.offset = offset;
			channels.push_back(channel);
			offset += 8;
		}
		// uv1
		if (channelMask & (1 << VertexAttribute::VA_UV1))
		{
			VertexChannelInfo channel = {};
			channel.attribute = VertexAttribute::VA_UV1;
			channel.format = VertexElementType::VET_Float2;
			channel.stream = 0;
			channel.offset = offset;
			channels.push_back(channel);
			offset += 8;
		}
		// color
		if (channelMask & (1 << VertexAttribute::VA_Color))
		{
			VertexChannelInfo channel = {};
			channel.attribute = VertexAttribute::VA_Color;
			channel.format = VertexElementType::VET_Float4;
			channel.stream = 0;
			channel.offset = offset;
			channels.push_back(channel);
			offset += 16;
		}
		// normal
		if (channelMask & (1 << VertexAttribute::VA_Normal))
		{
			VertexChannelInfo channel = {};
			channel.attribute = VertexAttribute::VA_Normal;
			channel.format = VertexElementType::VET_Float3;
			channel.stream = 0;
			channel.offset = offset;
			channels.push_back(channel);
			offset += 12;
		}
		// tangent
		if (channelMask & (1 << VertexAttribute::VA_Tangent))
		{
			VertexChannelInfo channel = {};
			channel.attribute = VertexAttribute::VA_Tangent;
			channel.format = VertexElementType::VET_Float3;
			channel.stream = 0;
			channel.offset = offset;
			channels.push_back(channel);
			offset += 12;
		}

		// 顶点数据
		uint32 vertexStreamSize = vertices.size() * sizeof(float);
		uint8* vertStreamData = new uint8[vertexStreamSize];
		std::memcpy(vertStreamData, vertices.data(), vertexStreamSize);

		// stream info
		VertexStreamInfo streamInfo;
		streamInfo.channelMask = channelMask;
		streamInfo.size = vertexStreamSize;

		// 索引数据
		uint32 indexStreamSize = uint32(indices.size() * sizeof(uint32));
		uint8* indexStreamData = new uint8[indexStreamSize];
		std::memcpy(indexStreamData, indices.data(), indexStreamSize);

		// make vertexbuffer
		std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>();
		vertexBuffer->AddStream(streamInfo, channels, vertStreamData);
		std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indexStreamData, indexStreamSize, PrimitiveType::PT_TriangleList, VkIndexType::VK_INDEX_TYPE_UINT32);
		
		// upload to gpu
		vertexBuffer->Upload();
		indexBuffer->Upload();

		// 创建Renderable
		std::shared_ptr<Renderable> renderable = std::make_shared<Renderable>(vertexBuffer, indexBuffer);
		renderables.push_back(renderable);
	}

	return renderables;
}
