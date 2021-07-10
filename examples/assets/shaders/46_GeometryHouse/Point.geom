#version 450

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (location = 0) out vec3 outColor;

in gl_PerVertex 
{
    vec4  gl_Position;
	float gl_PointSize;
} gl_in[];

void main(void)
{	
    gl_PointSize = 1.0;
    
    vec4 position = gl_in[0].gl_Position;
    float size  = 2;

    outColor = vec3(1.0, 0.0, 0.0);
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * (position + vec4(-size, -size, 0.0, 0.0));
    EmitVertex();  

    outColor = vec3(0.0, 1.0, 0.0);
    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * (position + vec4( size, -size, 0.0, 0.0));
    EmitVertex();

    outColor = vec3(0.0, 0.0, 1.0);
    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * (position + vec4(-size,  size, 0.0, 0.0));
    EmitVertex();

    outColor = vec3(1.0, 1.0, 0.0);
    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * (position + vec4( size,  size, 0.0, 0.0));
    EmitVertex();

    outColor = vec3(0.0, 1.0, 1.0);
    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * (position + vec4(0,  size * 2, 0.0, 0.0));
    EmitVertex();

    EndPrimitive();
}