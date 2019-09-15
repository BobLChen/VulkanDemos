#version 450

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput inputNormal;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput inputDepth;

layout (binding = 3) uniform ParamBlock
{
	int			attachmentIndex;
	float		zNear;
	float		zFar;
	float		one;
	float		xMaxFar;
	float		yMaxFar;
	vec2		padding;
} fragParam;

#define NUM_LIGHTS 64
struct PointLight {
	vec4 position;
	vec3 color;
	float radius;
};

layout (binding = 5) uniform LightDataBlock
{
	PointLight lights[NUM_LIGHTS];
} lightDatas;

layout (location = 0) in vec2 inUV0;
layout (location = 1) in vec4 inRay;

layout (location = 0) out vec4 outFragColor;

vec4 zBufferParams;

float Linear01Depth( float z )
{
	return 1.0 / (zBufferParams.x * z + zBufferParams.y);
}

float LinearEyeDepth( float z )
{
	return 1.0 / (zBufferParams.z * z + zBufferParams.w);
}

float DoAttenuation(float range, float d)
{
    return 1.0 - smoothstep(range * 0.75, range, d);
}

void main() 
{
	float zc0 = 1.0 - fragParam.zFar / fragParam.zNear;
	float zc1 = fragParam.zFar / fragParam.zNear;
	zBufferParams = vec4(zc0, zc1, zc0 / fragParam.zFar, zc1 / fragParam.zFar);

	// world position
	float depth   = subpassLoad(inputDepth).r;
	float realZ01 = Linear01Depth(depth);
	vec4 position = vec4(inRay.xyz * realZ01, 1.0);

	// normal [0, 1] -> [-1, 1]
	vec4 normal  = subpassLoad(inputNormal);
	normal = normal * 2 - 1;

	// albedo color
	vec4 albedo  = subpassLoad(inputColor);

	if (fragParam.attachmentIndex == 0) {
		vec4 ambient  = vec4(0.20);
		outFragColor  = vec4(0.0) + ambient;
		for (int i = 0; i < NUM_LIGHTS; ++i)
		{
			vec3 lightDir = lightDatas.lights[i].position.xyz - position.xyz;
			float dist    = length(lightDir);
			float atten   = DoAttenuation(lightDatas.lights[i].radius, dist);
			float ndotl   = max(0.0, dot(normal.xyz, normalize(lightDir)));
			vec3 diffuse  = lightDatas.lights[i].color * albedo.xyz * ndotl * atten;

			outFragColor.xyz += diffuse;
		}
	}
	else if (fragParam.attachmentIndex == 1) {
		outFragColor = albedo;
	} 
	else if (fragParam.attachmentIndex == 2) {
		outFragColor = position / 1500.0;
	}
	else if (fragParam.attachmentIndex == 3) {
		outFragColor = normal;
	}
	else {
		// undefined
	}
}