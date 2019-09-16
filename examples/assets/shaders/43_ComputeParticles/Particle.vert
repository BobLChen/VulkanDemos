#version 450

layout (location = 0) in vec4 inCustom0;
layout (location = 1) in vec4 inCustom1;

layout (location = 0) out float outGradient;

layout (binding = 0) uniform ParticleParam 
{
	vec4 data0;
    vec4 data1;
} param;

out gl_PerVertex 
{
    vec4  gl_Position;
    float gl_PointSize;
};

void main() 
{
    gl_PointSize = param.data1.x;
    outGradient  = inCustom1.w;
	gl_Position  = vec4(inCustom0.xy, 1.0, 1.0);
}