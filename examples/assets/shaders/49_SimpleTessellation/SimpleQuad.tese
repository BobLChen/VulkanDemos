#version 450

layout (binding = 1) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout(quads, equal_spacing, ccw) in;

layout (location = 0) out vec3 outColor;

in gl_PerVertex 
{
    vec4  gl_Position;
} gl_in[];

void main()
{
	float u   = gl_TessCoord.x;
	float v   = gl_TessCoord.y;

	gl_Position = v * (u * gl_in[0].gl_Position + (1 - u) * gl_in[1].gl_Position) + (1 - v) * (u * gl_in[3].gl_Position + (1 - u) * gl_in[2].gl_Position);
	
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * gl_Position;

    outColor = gl_TessCoord.xyz;
}