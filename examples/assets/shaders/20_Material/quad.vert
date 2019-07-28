#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec2 outUV0;
layout (location = 1) out vec4 outRay;

layout (binding = 3) uniform ParamBlock
{
	int			attachmentIndex;
	float		zNear;
	float		zFar;
	float		one;
	float		xMaxFar;
	float		yMaxFar;
	vec2		padding;
} cameraParam;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() 
{
	gl_Position = vec4(inPosition, 1.0f);

	vec4 param  = vec4(cameraParam.xMaxFar, cameraParam.yMaxFar, cameraParam.zFar, 1.0);
	
	outRay.x = inPosition.x * param.x;
	outRay.y = inPosition.y * param.y;
	outRay.z = param.z;
	outRay.w = param.w;
	
	outUV0   = inUV0;
}