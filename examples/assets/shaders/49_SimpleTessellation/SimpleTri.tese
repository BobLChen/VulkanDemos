#version 450

layout (binding = 1) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout(triangles, equal_spacing, ccw) in;

layout (location = 0) out vec3 outColor;

in gl_PerVertex 
{
    vec4  gl_Position;
} gl_in[];

void main()
{
	gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) + (gl_TessCoord.y * gl_in[1].gl_Position) + (gl_TessCoord.z * gl_in[2].gl_Position); 
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * gl_Position;

    outColor = gl_TessCoord.xyz;
}