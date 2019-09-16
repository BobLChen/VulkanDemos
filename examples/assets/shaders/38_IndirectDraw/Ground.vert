#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (binding = 1) uniform LightsMVPBlock 
{
	mat4 viewMatrix;
	vec4 cascadeScale[4];
	vec4 cascadeOffset[4];
	mat4 projMatrix[4];
	vec4 offset[4];
	vec4 direction;
	vec4 bias;
	vec4 debug;
} lightMVP;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec4 outLightViewPos;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	vec4 worldPos = uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);
	mat3 normalMatrix = transpose(inverse(mat3(uboMVP.modelMatrix)));
	vec3 normal = normalize(normalMatrix * inNormal.xyz);
	outNormal = normal;
	outColor  = inColor;
	outLightViewPos = lightMVP.viewMatrix * worldPos;

	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * worldPos;
}