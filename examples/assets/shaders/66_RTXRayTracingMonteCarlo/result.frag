#version 450

layout (binding = 1) uniform sampler2D diffuseMap;

layout (binding = 0) uniform ParamBlock 
{
	vec4 data;
} uboParam;

layout (location = 0) in vec2 inUV0;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = texture(diffuseMap, inUV0) / uboParam.data.x;
}