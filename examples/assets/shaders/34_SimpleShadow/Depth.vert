#version 450

layout (location = 0) in vec3 inPosition;
// 为了占位，真实环境中不要这样使用，单纯的一个Position即可
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec4 notUsed;
} uboMVP;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);
}