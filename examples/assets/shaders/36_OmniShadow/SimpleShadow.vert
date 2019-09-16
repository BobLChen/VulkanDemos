#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (binding = 1) uniform LightMVPBlock 
{
	vec4 position;
    vec4 bias;
} lightParam;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outLightDir;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	mat3 nrmMat33 = transpose(inverse(mat3(uboMVP.modelMatrix)));
	vec3 normal   = normalize(nrmMat33 * inNormal.xyz);
	outNormal     = normal;

	vec4 worldPos = uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);
	outLightDir   = worldPos.xyz - lightParam.position.xyz;
	
	gl_Position   = uboMVP.projectionMatrix * uboMVP.viewMatrix * worldPos;
}