#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform UBO 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} ubo;

layout (location = 0) out vec3 outNormal;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	outNormal = inNormal;
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}
