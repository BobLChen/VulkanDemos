#version 450

#define INSTANCE_COUNT 512

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
	mat4x4 transforms[INSTANCE_COUNT];
	vec4   colors[INSTANCE_COUNT];
} uboTransform;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	int instanceID  = int(inInstanceID);

	vec4 position   = uboTransform.transforms[instanceID] * vec4(inPosition, 1);
	
	outUV    = inUV0;
	outColor = uboTransform.colors[instanceID];
	
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * position;
}