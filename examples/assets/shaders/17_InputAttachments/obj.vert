#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform ViewProjBlock 
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboViewProj;

layout (binding = 1) uniform ModelDynamicBlock
{
	mat4 modelMatrix;
} uboModel;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outPosition;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	mat3 normalMatrix = transpose(inverse(mat3(uboModel.modelMatrix)));
	vec3 normal = normalize(normalMatrix * inNormal);

	outNormal   = normal;
	outPosition = (uboModel.modelMatrix * vec4(inPosition.xyz, 1.0)).xyz;

	gl_Position = uboViewProj.projectionMatrix * uboViewProj.viewMatrix * uboModel.modelMatrix * vec4(inPosition.xyz, 1.0);
}
