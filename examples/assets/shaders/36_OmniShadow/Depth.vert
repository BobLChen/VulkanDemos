#version 450

#extension GL_EXT_multiview : enable

layout (location = 0) in vec3 inPosition;
// 为了占位，真实环境中不要这样使用，单纯的一个Position即可
layout (location = 1) in vec2 inUV0;
layout (location = 2) in vec3 inNormal;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix[6];
	mat4 projectionMatrix;
	vec4 position;
} uboMVP;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	vec4 worldPos = uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);
	gl_Position   = uboMVP.projectionMatrix * uboMVP.viewMatrix[gl_ViewIndex] * worldPos;
	gl_Position.z = length(uboMVP.position.xyz - worldPos.xyz);
	gl_Position.w = uboMVP.position.w;
}