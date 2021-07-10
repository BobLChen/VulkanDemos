#version 450

layout (binding = 0) uniform TessParam 
{
    vec4 levelOuter;
    vec4 levelInner;
} tessParam;

layout (vertices = 2) out;

in gl_PerVertex 
{
    vec4  gl_Position;
} gl_in[];

out gl_PerVertex 
{
    vec4 gl_Position;
} gl_out[];

void main()
{
	if (gl_InvocationID == 0)
	{
		gl_TessLevelInner[0] = tessParam.levelInner.x;
        gl_TessLevelInner[1] = tessParam.levelInner.y;

		gl_TessLevelOuter[0] = tessParam.levelOuter.x;
		gl_TessLevelOuter[1] = tessParam.levelOuter.y;
		gl_TessLevelOuter[2] = tessParam.levelOuter.z;
        gl_TessLevelOuter[3] = tessParam.levelOuter.w;		
	}
    
	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
} 
