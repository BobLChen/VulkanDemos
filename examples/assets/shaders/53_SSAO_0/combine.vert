#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec2 outUV0;
layout (location = 1) out vec4 outRay;

layout (binding = 4) uniform ParamBlock
{
	vec4 data;
	mat4 view;
	mat4 invView;
	mat4 proj;
} param;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	gl_Position = vec4(inPosition, 1.0f);

	float zNear   = param.data.x;
	float zFar    = param.data.y;
	float xMaxFar = param.data.z;
	float yMaxFar = param.data.w;
	
	outRay.x = inPosition.x * xMaxFar;
	outRay.y = inPosition.y * yMaxFar;
	outRay.z = zFar;
	outRay.w = 1.0;
	
	outUV0   = inUV0;
}