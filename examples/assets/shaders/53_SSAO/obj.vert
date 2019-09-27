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

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out float outDepth;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	mat3 normalMatrix = transpose(inverse(mat3(uboMVP.modelMatrix)));
	vec3 normal  = normalize(normalMatrix * inNormal.xyz);
	
	outUV       = inUV0;
	outNormal   = normal;
	
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(inPosition.xyz, 1.0);

	outDepth    = gl_Position.z / gl_Position.w;
}