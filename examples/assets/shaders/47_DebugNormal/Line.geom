#version 450

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

layout (location = 0) in vec3 inNormal[];
layout (location = 0) out vec3 outColor;

in gl_PerVertex 
{
    vec4  gl_Position;
} gl_in[];

void main(void)
{	
    float normalLength = 1;
	for(int i = 0; i < gl_in.length(); ++i)
	{
		vec3 position = gl_in[i].gl_Position.xyz;
		vec3 normal   = inNormal[i].xyz;

		gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(position, 1.0);
		outColor = vec3(0.0, 1.0, 0.0);
		EmitVertex();

		gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(position + normalize(normal) * normalLength, 1.0);
		outColor = vec3(0.0, 0.0, 1.0);
		EmitVertex();

		EndPrimitive();
	}
}