#version 450

layout (set = 0, binding = 1) uniform sampler2D samplerColorMap;
layout (set = 0, binding = 2) uniform sampler2D samplerNormalMap;
layout (set = 0, binding = 3) uniform sampler2D samplerSpecularMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 lightDir = vec3(0, 0, 1);
	vec3 normal = normalize(inNormal);
	vec4 diffuse = texture(samplerColorMap, inUV0);
	vec4 normalmap = texture(samplerNormalMap, inUV0) * 2.0 - 1.0;
	vec4 specular = texture(samplerSpecularMap, inUV0);
    float ndl = max(dot(normal.rgb, lightDir), 0.0);

	outFragColor.rgb = vec3(ndl);
}