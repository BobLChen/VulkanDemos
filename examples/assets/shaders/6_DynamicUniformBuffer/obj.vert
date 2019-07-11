#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform Model 
{
	mat4 modelMatrix;
} model;

layout (binding = 1) uniform ViewProjection
{
	vec4 color;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} viewProjection;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec4 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	outNormal   = inNormal;
	outColor    = viewProjection.color;
	gl_Position = viewProjection.projectionMatrix * viewProjection.viewMatrix * model.modelMatrix * vec4(inPosition.xyz, 1.0);
}
