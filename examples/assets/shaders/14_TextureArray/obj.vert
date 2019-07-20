#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec4 inTangent;
layout (location = 4) in vec4 inCustom;

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
layout (location = 2) out vec3 outTangent;
layout (location = 3) out vec3 outBiTangent;
layout (location = 4) out vec4 outCustom;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	mat3 normalMatrix = transpose(inverse(mat3(uboModel.modelMatrix)));

	vec3 normal  = normalize(normalMatrix * inNormal.xyz);
	vec3 tangent = normalize(normalMatrix * inTangent.xyz);
	
	outUV0       = inUV0;
	outNormal    = normal;
	outTangent   = tangent;
	outBiTangent = cross(normal, tangent) * inTangent.w;
	outCustom    = inCustom;

	gl_Position = uboViewProj.projectionMatrix * uboViewProj.viewMatrix * uboModel.modelMatrix * vec4(inPosition.xyz, 1.0);
}
