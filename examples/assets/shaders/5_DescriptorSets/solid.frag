#version 450

layout (set = 0, binding = 1) uniform sampler2D samplerColorMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 lightDir = vec3(0, -1, 0);
    float diffuse = dot(normalize(inNormal), lightDir);
	outFragColor.rgb = texture(samplerColorMap, inUV0).rgb * diffuse;
}