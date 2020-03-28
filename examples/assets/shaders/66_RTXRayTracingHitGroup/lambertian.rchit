#version 460

#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "common.glsl"

layout(location = 0) rayPayloadInNV RayPayloadInfo rayInfo;

struct Vertex
{
    float vertices[11];
};

struct Material
{
    vec4 albedo;
	vec4 params; // roughness, metallic, occlusion
	ivec4 textureIDs; // albedo, roughness, metallic
};

struct Triangle 
{
    Vertex v0;
    Vertex v1;
    Vertex v2;
};

struct Face
{
    uint indices[3];
};

layout(std430, binding = 0, set = 1) buffer Vertices
{
    Vertex datas[];
} vertices[];

layout(std430, binding = 1, set = 1) buffer Indices
{
    Face datas[];
} indices[];

layout(std430, binding = 2, set = 1) buffer Materials
{
    Material datas[];
} materials;

layout(std430, binding = 3, set = 1) buffer ObjectInstances
{
    ivec4 datas[];
} objects;

layout(binding = 4, set = 1) uniform sampler2D textures[];

hitAttributeNV vec3 attribs;

vec2 Blerp(vec2 b, vec2 p1, vec2 p2, vec2 p3)
{
    return (1.0 - b.x - b.y) * p1 + b.x * p2 + b.y * p3;
}

vec3 Blerp(vec2 b, vec3 p1, vec3 p2, vec3 p3)
{
    return (1.0 - b.x - b.y) * p1 + b.x * p2 + b.y * p3;
}

Triangle FetchTriangle(uint geometryIndex, uint faceIndex)
{
    ivec4 object = objects.datas[geometryIndex];

    Face face = indices[object.y].datas[faceIndex];

    Triangle result;
    result.v0 = vertices[object.y].datas[face.indices[0]];
    result.v1 = vertices[object.y].datas[face.indices[1]];
    result.v2 = vertices[object.y].datas[face.indices[2]];

    return result;
}

vec3 FetchNormal(Triangle triangle, vec2 b)
{
    vec3 normal0 = vec3(triangle.v0.vertices[5], triangle.v0.vertices[6], triangle.v0.vertices[7]);
    vec3 normal1 = vec3(triangle.v1.vertices[5], triangle.v1.vertices[6], triangle.v1.vertices[7]);
    vec3 normal2 = vec3(triangle.v2.vertices[5], triangle.v2.vertices[6], triangle.v2.vertices[7]);
    return Blerp(b, normal0, normal1, normal2);
}

vec2 FetchUV(Triangle triangle, vec2 b)
{
    vec2 uv0 = vec2(triangle.v0.vertices[3], triangle.v0.vertices[4]);
    vec2 uv1 = vec2(triangle.v1.vertices[3], triangle.v1.vertices[4]);
    vec2 uv2 = vec2(triangle.v2.vertices[3], triangle.v2.vertices[4]);
    return Blerp(b, uv0, uv1, uv2);
}

Material FetchMaterial(uint geometryIndex)
{
    ivec4 object = objects.datas[geometryIndex];

    return materials.datas[object.x];
}

float Schlick(const float cosine, const float refractionIndex)
{
	float r0 = (1 - refractionIndex) / (1 + refractionIndex);
	r0 *= r0;
	return r0 + (1 - r0) * pow(1 - cosine, 5);
}

RayPayloadInfo Scatter(const vec3 color, const vec3 direction, const vec3 normal, const float t, inout uint seed)
{
    const bool isScattered = dot(direction, normal) < 0;
	const vec4 colorAndDistance = vec4(color, t);
	const vec4 scatter = vec4(normal + RandomInUnitSphere(seed), isScattered ? 1 : 0);
    
    RayPayloadInfo rayInfo;
    rayInfo.colorAndDistance = colorAndDistance;
    rayInfo.scatterDirection = scatter;
    rayInfo.randomSeed = seed;
    
    return rayInfo;
}

void main()
{
    Triangle triangle = FetchTriangle(gl_InstanceID, gl_PrimitiveID);
    Material material = FetchMaterial(gl_InstanceID);
    
    vec3 lightDir = normalize(vec3(0, 1, 1));
    vec3 normal = FetchNormal(triangle, attribs.xy);
    float NdotL = max(dot(lightDir, normal), 0);
    vec2 uv = FetchUV(triangle, attribs.xy);

    vec3 diffuse = vec3(0, 0, 0);
    if (material.textureIDs.x >= 0) {
        diffuse = texture(textures[material.textureIDs.x], uv).xyz;
    }
    else {
        diffuse = material.albedo.xyz;
    }
    
    rayInfo = Scatter(diffuse, gl_WorldRayDirectionNV, normal, gl_HitTNV, rayInfo.randomSeed);
}