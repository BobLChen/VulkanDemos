#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;
layout (location = 2) in vec3 inNormal;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (binding = 1) uniform PrevMVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboPrevMVP;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;

layout (location = 2) out vec2 outVelocity;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	mat3 normalMatrix = transpose(inverse(mat3(uboMVP.modelMatrix)));
	vec3 normal  = normalize(normalMatrix * inNormal.xyz);
	
	outUV        = inUV0;
	outNormal    = normal;

	vec4 currPosition;
	vec4 prevPosition;
	
	currPosition = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);
	prevPosition = uboPrevMVP.projectionMatrix * uboPrevMVP.viewMatrix * uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);

	gl_Position  = currPosition;

	currPosition /= currPosition.w;
	prevPosition /= prevPosition.w;

	vec2 velocity = currPosition.xy - prevPosition.xy;
	velocity /= 2.0;

	outVelocity = velocity;
}