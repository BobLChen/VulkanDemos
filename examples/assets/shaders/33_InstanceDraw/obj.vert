#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec4 inInstanceDualQuat0;
layout (location = 4) in vec4 inInstanceDualQuat1;

layout (binding = 0) uniform ViewProjBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV0;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

vec3 DualQuatTransformPosition(mat2x4 dualQuat, vec3 position)
{
	float len = length(dualQuat[0]);
	dualQuat /= len;
	
	vec3 result = position.xyz + 2.0 * cross(dualQuat[0].xyz, cross(dualQuat[0].xyz, position.xyz) + dualQuat[0].w * position.xyz);
	vec3 trans  = 2.0 * (dualQuat[0].w * dualQuat[1].xyz - dualQuat[1].w * dualQuat[0].xyz + cross(dualQuat[0].xyz, dualQuat[1].xyz));
	result += trans;

	return result;
}

vec3 DualQuatTransformVector(mat2x4 dualQuat, vec3 vector)
{
	return vector + 2.0 * cross(dualQuat[0].xyz, cross(dualQuat[0].xyz, vector) + dualQuat[0].w * vector);
}

void main() 
{
	mat2x4 dualQuat;
	dualQuat[0] = inInstanceDualQuat0;
	dualQuat[1] = inInstanceDualQuat1;
	vec4 position = vec4(DualQuatTransformPosition(dualQuat, inPosition.xyz), 1.0);
	vec3 normal   = DualQuatTransformVector(dualQuat, inNormal);

	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * position;
	outNormal = normal;
	outUV0	  = inUV0;
}
