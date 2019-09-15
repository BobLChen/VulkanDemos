#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (binding = 1) uniform LightMVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec4 direction;
} lightMVP;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outShadowCoord;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	vec4 worldPos   = uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);
	vec4 shadowProj = lightMVP.projectionMatrix * lightMVP.viewMatrix * worldPos;
	outShadowCoord.xyz = shadowProj.xyz / shadowProj.w;
	// [-1, 1] -> [0, 1]
	outShadowCoord.xy = outShadowCoord.xy * 0.5 + 0.5;
	// flip y
	outShadowCoord.y = 1.0 - outShadowCoord.y;

	mat3 normalMatrix = transpose(inverse(mat3(uboMVP.modelMatrix)));
	vec3 normal  = normalize(normalMatrix * inNormal.xyz);
	outNormal    = normal;

	gl_Position  = uboMVP.projectionMatrix * uboMVP.viewMatrix * worldPos;
}