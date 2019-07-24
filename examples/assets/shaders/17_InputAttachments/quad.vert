#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;

layout (binding = 3) uniform CameraParamBlock
{
	float xMaxFar;
	float yMaxFar;
	float zFar;
	float one;
} cameraParam;

layout (location = 0) out vec2 outUV0;
layout (location = 1) out vec4 outEye;

out gl_PerVertex {
	vec4 gl_Position;
};

void main() 
{
	gl_Position = vec4(inPosition, 1.0f);

	outUV0   = inUV0;

	outEye.x = inPosition.x * cameraParam.xMaxFar;
	outEye.y = inPosition.y * cameraParam.yMaxFar;
	outEye.z = cameraParam.zFar;
	outEye.w = cameraParam.one;
}