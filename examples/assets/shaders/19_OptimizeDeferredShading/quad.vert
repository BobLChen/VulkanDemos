#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec2 outUV0;
layout (location = 1) out vec4 outRay;

layout (binding = 3) uniform ParamBlock
{
	vec4 param0;	// (attachmentIndex, zNear, zFar, one)
	vec4 param1;	// (xMaxFar, yMaxFar, padding, padding)
	mat4 invView;
} paramData;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() 
{
	gl_Position = vec4(inPosition, 1.0f);

	float xMaxFar = paramData.param1.x;
	float yMaxFar = paramData.param1.y;
	float zFar    = paramData.param0.z;
	
	outRay.x = inPosition.x * xMaxFar;
	outRay.y = inPosition.y * yMaxFar;
	outRay.z = zFar;
	outRay.w = 1.0;
	
	outUV0   = inUV0;
}