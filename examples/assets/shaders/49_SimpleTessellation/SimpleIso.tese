#version 450

layout (binding = 1) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout(isolines, equal_spacing, cw) in;

layout (location = 0) out vec3 outColor;

in gl_PerVertex 
{
    vec4  gl_Position;
} gl_in[];

void main()
{
    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
    
    gl_Position = mix(p1, p2, gl_TessCoord.y);
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * gl_Position;

    outColor = gl_TessCoord.xyz;
}