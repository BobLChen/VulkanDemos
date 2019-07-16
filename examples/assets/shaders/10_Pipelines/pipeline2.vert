#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (binding = 1) uniform ParamBlock 
{
	float intensity;
} uboParam;

layout (location = 0) out vec3 outNormal;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
    vec3 normal   = normalize(inNormal);
    vec3 position = inPosition + inNormal * uboParam.intensity;
    outNormal     = inNormal;
	gl_Position   = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(position.xyz, 1.0);
}
