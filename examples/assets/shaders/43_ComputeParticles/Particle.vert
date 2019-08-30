#version 450

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inUV0;

layout (location = 0) out float outGradient;

out gl_PerVertex 
{
    vec4  gl_Position;
    float gl_PointSize;
};

void main() 
{
    gl_PointSize = 8.0;
    outGradient  = inPosition.z;
	gl_Position  = vec4(inPosition.xy, 1.0, 1.0);
}