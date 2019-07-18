#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;
layout (location = 2) in vec3 inNormal;

layout (binding = 0) uniform ViewProjBlock 
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboViewProj;

layout (binding = 1) uniform ModelBlock
{
	mat4 modelMatrix;
} uboModel;

layout (location = 0) out vec2 outUV0;
layout (location = 1) out vec3 outNormal;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	mat3 normalMatrix = transpose(inverse(mat3(uboModel.modelMatrix)));
	vec3 normal  = normalize(normalMatrix * inNormal);

	outUV0 = inUV0;
	outNormal = normal;

	gl_Position = uboViewProj.projectionMatrix * uboViewProj.viewMatrix * uboModel.modelMatrix * vec4(inPosition.xyz, 1.0);
}
