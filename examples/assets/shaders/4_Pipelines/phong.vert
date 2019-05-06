#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform UBO 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outViewDir;
layout (location = 2) out vec3 outLightDir;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);

	outLightDir = vec3(0, 50, -50) - inPosition;
	outViewDir  = vec3(0, 0, -1);		
	outNormal   = inNormal;
}