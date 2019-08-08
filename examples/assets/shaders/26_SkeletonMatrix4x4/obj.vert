#version 450

layout (location = 0) in vec3  inPosition;
layout (location = 1) in vec2  inUV0;
layout (location = 2) in vec3  inNormal;
layout (location = 3) in vec4  inSkinIndex;
layout (location = 4) in vec4  inSkinWeight;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

#define MAX_BONES 64

layout (binding = 1) uniform BonesTransformBlock 
{
	mat4 bones[MAX_BONES];
} bonesData;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec4 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{

	mat4 boneMatrix = bonesData.bones[int(inSkinIndex.x)] * inSkinWeight.x;
	boneMatrix += bonesData.bones[int(inSkinIndex.y)] * inSkinWeight.y;
	boneMatrix += bonesData.bones[int(inSkinIndex.z)] * inSkinWeight.z;
	boneMatrix += bonesData.bones[int(inSkinIndex.w)] * inSkinWeight.w;

	mat4 modeMatrix   = uboMVP.modelMatrix * boneMatrix;
	mat3 normalMatrix = transpose(inverse(mat3(modeMatrix)));

	vec3 normal = normalize(normalMatrix * inNormal.xyz);
	
	outUV       = inUV0;
	outNormal   = normal;
	outColor    = inSkinWeight;
	
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * modeMatrix * vec4(inPosition.xyz, 1.0);
}