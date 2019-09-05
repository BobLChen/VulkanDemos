#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;

out gl_PerVertex 
{
    vec4 gl_Position;
};

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV0;

void main()
{
	gl_Position = vec4(inPosition, 1.0);
    outNormal = inNormal;
    outUV0 = inUV0;
}