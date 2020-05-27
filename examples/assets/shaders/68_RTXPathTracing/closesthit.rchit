#version 460

#extension GL_NV_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : require

#include "lib/Common.glsl"
#include "lib/ShadingCommon.glsl"

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
    vec3 normal = Blerp(b, normal0, normal1, normal2);

    return gl_ObjectToWorldNV * vec4(normal, 0.0);
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

vec3 GammaToLinearSpace(vec3 sRGB)
{
    // Approximate version from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
}

void main()
{
    Triangle triangle = FetchTriangle(gl_InstanceID, gl_PrimitiveID);
    Material material = FetchMaterial(gl_InstanceID);
    
    vec3 normal = FetchNormal(triangle, attribs.xy);
    vec2 uv = FetchUV(triangle, attribs.xy);

    vec3 albedo = vec3(0, 0, 0);
    if (material.textureIDs.x >= 0) {
        albedo = GammaToLinearSpace(texture(textures[material.textureIDs.x], uv).xyz);
    }
    else {
        albedo = material.albedo.xyz;
    }

    float roughness = material.params.x;
    if (material.textureIDs.y >= 0) {
        roughness = texture(textures[material.textureIDs.y], uv).x;
    }

    float metallic  = material.params.y;
    if (material.textureIDs.z >= 0) {
        roughness = texture(textures[material.textureIDs.z], uv).x;
    }

    float specular = 0.5;
    rayInfo.hitT = gl_HitTNV;
    rayInfo.worldPos = gl_WorldRayOriginNV.xyz + gl_WorldRayDirectionNV.xyz * gl_HitTNV;
    rayInfo.worldNormal = normal;
    rayInfo.radiance = vec3(0.0, 0.0, 0.0);
    rayInfo.baseColor = albedo;
    rayInfo.roughness = roughness;
    rayInfo.metallic = metallic;
    rayInfo.ior = 1.5;
    rayInfo.opacity = 1.0;
    rayInfo.shadingMode = 0.0;
    rayInfo.diffuseColor = albedo - albedo * metallic;
    rayInfo.specularColor = ComputeF0(specular, albedo, metallic);
}

// accelerationStructureNV type -> OpTypeAccelerationStructureNV instruction

// rayPayloadNV storage qualifier -> RayPayloadNV storage class
// rayPayloadInNV storage qualifier -> IncomingRayPayloadNV storage class
// hitAttributeNV storage qualifier -> HitAttributeNV storage class
// callableDataNV storage qualifier -> CallableDataNV storage class
// callableDataInNV storage qualifier -> IncomingCallableDataNV storage class

// shaderRecordNV decorated buffer block -> ShaderRecordBufferNV storage class

// gl_LaunchIDNV -> LaunchIdNV decorated OpVariable
// gl_LaunchSizeNV -> LaunchSizeNV decorated OpVariable
// gl_PrimitiveID -> PrimitiveId decorated OpVariable
// gl_InstanceID -> InstanceId decorated OpVariable
// gl_InstanceCustomIndexNV -> InstanceCustomIndexNV decorated OpVariable
// gl_WorldRayOriginNV -> WorldRayOriginNV decorated OpVariable
// gl_WorldRayDirectionNV -> WorldRayDirectionNV decorated OpVariable
// gl_ObjectRayOriginNV -> ObjectRayOriginNV decorated OpVariable
// gl_ObjectRayDirectionNV -> ObjectRayDirectionNV decorated OpVariable
// gl_RayTminNV -> RayTminNV decorated OpVariable
// gl_RayTmaxNV -> RayTmaxNV decorated OpVariable
// gl_IncomingRayFlagsNV -> IncomingRayFlagsNV decorated OpVariable
// gl_HitTNV -> HitTNV decorated OpVariable
// gl_HitKindNV -> HitKindNV decorated OpVariable
// gl_ObjectToWorldNV -> ObjectToWorldNV decorated OpVariable
// gl_WorldToObjectNV -> WorldToObjectNV decorated OpVariable

// gl_RayFlagsNoneNV -> constant, no semantic needed
// gl_RayFlagsOpaqueNV -> constant, no semantic needed
// gl_RayFlagsNoOpaqueNV -> constant, no semantic needed
// gl_RayFlagsTerminateOnFirstHitNV -> constant, no semantic needed
// gl_RayFlagsSkipClosestHitShaderNV -> constant, no semantic needed
// gl_RayFlagsCullBackFacingTrianglesNV -> constant, no semantic needed
// gl_RayFlagsCullFrontFacingTrianglesNV -> constant, no semantic needed
// gl_RayFlagsCullOpaqueNV -> constant, no semantic needed
// gl_RayFlagsCullNoOpaqueNV -> constant, no semantic needed

// traceNV -> OpTraceNV instruction
// reportIntersectionNV -> OpReportIntersectionNV instruction
// ignoreIntersectionNV -> OpIgnoreIntersectionNV instruction
// terminateRayNV -> OpTerminateRayNV instruction
// executeCallableNV -> OpExecuteCallableNV instruction