#version 450

#extension GL_EXT_multiview : enable

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix[6];
	mat4 projectionMatrix;
	vec4 position;
} uboMVP;

layout (location = 0) out float outLength;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	vec4 worldPos = uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);
	gl_Position   = uboMVP.projectionMatrix * uboMVP.viewMatrix[gl_ViewIndex] * worldPos;
	outLength     = length(uboMVP.position.xyz - worldPos.xyz);
}