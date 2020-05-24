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

vec3 CosineSampleHemisphere(float u1, float u2)
{
	vec3 dir;
	float r = sqrt(u1);
	float phi = 2.0 * PI * u2;
	dir.x = r * cos(phi);
	dir.y = r * sin(phi);
	dir.z = sqrt(max(0.0, 1.0 - dir.x * dir.x - dir.y * dir.y));
	return dir;
}

float GTR2(float NDotH, float a)
{
	float a2 = a * a;
	float t  = 1.0 + (a2 - 1.0) * NDotH * NDotH;
	return a2 / (PI * t * t);
}

float SchlickFresnel(float u)
{
	float m = clamp(1.0 - u, 0.0, 1.0);
	float m2 = m * m;
	return m2 * m2 * m;
}

float SmithG_GGX(float NDotv, float alphaG)
{
	float a = alphaG * alphaG;
	float b = NDotv * NDotv;
	return 1.0 / (NDotv + sqrt(a + b - a * b));
}

// http://www.pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/Sampling_Reflection_Functions.html
float Sample_Pdf(inout RayPayloadInfo rayInfo, float roughness, float metallic, vec3 V, vec3 N, vec3 L)
{
	float specularAlpha = max(0.0001, roughness);
	float diffuseRatio  = 0.5 * (1.0 - metallic);
	float specularRatio = 1.0 - diffuseRatio;

	vec3 halfVec   = normalize(L + V);
	float cosTheta = abs(dot(halfVec, N));
	float pdfGTR2  = GTR2(cosTheta, specularAlpha) * cosTheta;

	float pdfSpec = pdfGTR2 / (4.0 * abs(dot(L, halfVec)));
	float pdfDiff = abs(dot(L, N)) * (1.0 / PI);

	return diffuseRatio * pdfDiff + specularRatio * pdfSpec;
}

// http://www.pbr-book.org/3ed-2018/Light_Transport_I_Surface_Reflection/Sampling_Reflection_Functions.html
vec3 Sample_Wi(inout RayPayloadInfo rayInfo, float roughness, float metallic, vec3 V, vec3 N)
{
	vec3 dir = vec3(0, 0, 1);

	float probability  = Rand(rayInfo);
	float diffuseRatio = 0.5 * (1.0 - metallic);

	float r1 = Rand(rayInfo);
	float r2 = Rand(rayInfo);

	vec3 UpVector = abs(N.z) < 0.999 ? vec3(0, 0, 1) : vec3(1, 0, 0);
	vec3 TangentX = normalize(cross(UpVector, N));
	vec3 TangentY = cross(N, TangentX);

	if (probability < diffuseRatio)
	{
		dir = CosineSampleHemisphere(r1, r2);
		dir = TangentX * dir.x + TangentY * dir.y + N * dir.z;
	}
	else
	{
		float a = max(0.001, roughness);

		float phi = r1 * 2.0 * PI;

		float cosTheta = sqrt((1.0 - r2) / (1.0 + (a * a - 1.0) * r2));
		float sinTheta = clamp(sqrt(1.0 - (cosTheta * cosTheta)), 0.0, 1.0);

		float sinPhi = sin(phi);
		float cosPhi = cos(phi);

		vec3 halfVec = vec3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
		halfVec = TangentX * halfVec.x + TangentY * halfVec.y + N * halfVec.z;

		dir = 2.0 * dot(V, halfVec) * halfVec - V;
	}

	return dir;
}

vec3 BRDF(inout RayPayloadInfo rayInfo, vec3 albedo, float roughness, float metallic, vec3 V, vec3 N, vec3 L)
{
	float NDotL = dot(N, L);
	float NDotV = dot(N, V);

	if (NDotL <= 0.0 || NDotV <= 0.0) {
        return vec3(0.0);
    }

	vec3 H = normalize(L + V);

	float NDotH = dot(N, H);
	float LDotH = dot(L, H);

	// specular	
	float specular = 0.5;
	vec3 specularCol = mix(vec3(1.0) * 0.08 * specular, albedo.xyz, metallic);

	float a  = max(0.001, roughness);
	float Ds = GTR2(NDotH, a);
	float FH = SchlickFresnel(LDotH);
	vec3 Fs  = mix(specularCol, vec3(1.0), FH);
	float roughg = (roughness * 0.5 + 0.5);
	roughg = roughg * roughg;
	float Gs = SmithG_GGX(NDotL, roughg) * SmithG_GGX(NDotV, roughg);

	return (albedo.xyz / PI) * (1.0 - metallic) + Gs * Fs * Ds;
}

RayPayloadInfo Scatter(const vec3 albedo, float roughness, float metallic, const vec3 direction, const vec3 normal, const float t)
{
    vec3 wi = Sample_Wi(rayInfo, roughness, metallic, -direction, normal);
    float pdf = Sample_Pdf(rayInfo, roughness, metallic, -direction, normal, wi);

    RayPayloadInfo retInfo;

    if (pdf > 0.0) 
    {
        // http://www.pbr-book.org/3ed-2018/Monte_Carlo_Integration/The_Monte_Carlo_Estimator.html
        vec3 color = BRDF(rayInfo, albedo, roughness, metallic, -direction, normal, wi) * abs(dot(wi, normal)) / pdf;
        retInfo.colorAndDistance = vec4(color, t);
        retInfo.scatterDirection = vec4(wi, 1);
    }
    else
    {
        retInfo.colorAndDistance = vec4(0, 0, 0, -1);
        retInfo.scatterDirection = vec4(0, 0, 1, 0);
    }
    
    retInfo.seedAndRandom = rayInfo.seedAndRandom;
    
    return retInfo;
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

    rayInfo = Scatter(albedo, roughness, metallic, gl_WorldRayDirectionNV, normal, gl_HitTNV);
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