#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out vec2 outUV0;

out gl_PerVertex 
{
    vec4 gl_Position;
};

void main() 
{
    outUV0 = inUV0;
	gl_Position = vec4(inPosition, 1.0);
}