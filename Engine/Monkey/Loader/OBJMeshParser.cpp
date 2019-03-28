#include "File/FileManager.h"

#include "OBJMeshParser.h"
#include "tiny_obj_loader.h"

std::vector<std::shared_ptr<Renderable>> OBJMeshParser::LoadFromFile(const std::string& filename)
{
	tinyobj::attrib_t                attrib;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      warn;
	std::string                      err;

	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, FileManager::GetFilePath(filename).c_str());

	std::vector<std::shared_ptr<Renderable>> renderables;

	for (size_t s = 0; s < shapes.size(); ++s)
	{
		std::vector<float> vertices;
		std::vector<uint16> indices;
		size_t index_offset = 0;
		uint32 channelMask = 0;

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f)
		{
			// 不支持Quad
			int fv = shapes[s].mesh.num_face_vertices[f];
			if (fv >= 4)
			{
				continue;
			}
			// 遍历数据
			for (size_t v = 0; v < fv; v++)
			{
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				// position
				if (3 * idx.vertex_index + 2 < attrib.vertices.size())
				{
					channelMask = channelMask | (1 << VertexAttribute::VA_Position);
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					vertices.push_back(vx);
					vertices.push_back(vy);
					vertices.push_back(vz);
				}
				// uv
				if (2 * idx.texcoord_index + 1 < attrib.texcoords.size())
				{
					channelMask = channelMask | (1 << VertexAttribute::VA_UV0);
					tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
					vertices.push_back(tx);
					vertices.push_back(1.0f - ty);
				}
				// normal
				if (3 * idx.normal_index + 2 < attrib.normals.size())
				{
					channelMask = channelMask | (1 << VertexAttribute::VA_Normal);
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
					vertices.push_back(nx);
					vertices.push_back(ny);
					vertices.push_back(nz);
				}
				indices.push_back(indices.size());
			}
			index_offset += fv;
		}

		std::vector<VertexChannelInfo> channels;
		int32 offset = 0;
		// position
		if (channelMask | (1 << VertexAttribute::VA_Position))
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
		if (channelMask | (1 << VertexAttribute::VA_UV0))
		{
			VertexChannelInfo channel = {};
			channel.attribute = VertexAttribute::VA_UV0;
			channel.format = VertexElementType::VET_Float2;
			channel.stream = 0;
			channel.offset = offset;
			channels.push_back(channel);
			offset += 8;
		}
		// normal
		if (channelMask | (1 << VertexAttribute::VA_Normal))
		{
			VertexChannelInfo channel = {};
			channel.attribute = VertexAttribute::VA_Normal;
			channel.format = VertexElementType::VET_Float3;
			channel.stream = 0;
			channel.offset = offset;
			channels.push_back(channel);
			offset += 12;
		}

		// 顶点数据
		uint8* vertStreamData = new uint8[vertices.size() * sizeof(float)];
		std::memcpy(vertStreamData, vertices.data(), vertices.size() * sizeof(float));
		VertexStreamInfo streamInfo;
		streamInfo.channelMask = channelMask;
		streamInfo.size = uint32(vertices.size() * sizeof(float));
		std::shared_ptr<VertexBuffer> vertexBuffer = std::make_shared<VertexBuffer>();
		vertexBuffer->AddStream(streamInfo, channels, vertStreamData);
		// 索引数据
		uint32 indexStreamSize = uint32(indices.size() * sizeof(uint16));
		uint8* indexStreamData = new uint8[indexStreamSize];
		std::memcpy(indexStreamData, indices.data(), indexStreamSize);
		std::shared_ptr<IndexBuffer> indexBuffer = std::make_shared<IndexBuffer>(indexStreamData, indexStreamSize, PrimitiveType::PT_TriangleList, VkIndexType::VK_INDEX_TYPE_UINT16);
		// 创建Renderable
		renderables.push_back(std::make_shared<Renderable>(vertexBuffer, indexBuffer));
	}

	return renderables;
}
