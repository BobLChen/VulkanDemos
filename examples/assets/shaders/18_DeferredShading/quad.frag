#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputNormal;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput inputPosition;
layout (input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput inputDepth;

struct PointLight {
	vec4 position;
	vec4 colorAndRadius;
};

#define NUM_LIGHTS 64

layout (binding = 4) uniform ParamBlock
{
	PointLight lights[NUM_LIGHTS];
} lightDatas;

layout (location = 0) in vec2 inUV0;

layout (location = 0) out vec4 outFragColor;

float DoAttenuation(float range, float d)
{
    return 1.0 - smoothstep(range * 0.75, range, d);
}

void main() 
{
	vec4 albedo   = subpassLoad(inputColor);
	vec4 normal   = subpassLoad(inputNormal);
	vec4 position = subpassLoad(inputPosition);
	
	normal.xyz    = normalize(normal.xyz);
	
	vec4 ambient  = vec4(0.20);
	
	outFragColor  = vec4(0.0) + ambient;
	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		vec3 lightDir = lightDatas.lights[i].position.xyz - position.xyz;
		float dist    = length(lightDir);
		float atten   = DoAttenuation(lightDatas.lights[i].colorAndRadius.w, dist);
		float ndotl   = max(0.0, dot(normal.xyz, normalize(lightDir)));
		vec3 diffuse  = lightDatas.lights[i].colorAndRadius.xyz * albedo.xyz * ndotl * atten;

		outFragColor.xyz += diffuse;
	}
}