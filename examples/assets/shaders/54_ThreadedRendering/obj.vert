#version 450

#define INSTANCE_COUNT 1024

layout (location = 0) in vec3  inPosition;
layout (location = 1) in vec2  inUV0;
layout (location = 2) in float inInstanceID;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (binding = 2) uniform TransformBlock 
{
	mat2x4 transforms[INSTANCE_COUNT];
	vec4   colors[INSTANCE_COUNT];
} uboTransform;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

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

void main() 
{
	int instanceID  = int(inInstanceID);
	mat2x4 dualQuat = uboTransform.transforms[instanceID];

	vec4 position   = vec4(DualQuatTransformPosition(dualQuat, inPosition.xyz), 1.0);
	
	outUV    = inUV0;
	outColor = uboTransform.colors[instanceID];
	
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * position;
}