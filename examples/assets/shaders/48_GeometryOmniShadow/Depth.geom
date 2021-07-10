#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix[6];
	mat4 projectionMatrix;
	vec4 position;
} uboMVP;

in gl_PerVertex 
{
    vec4  gl_Position;
} gl_in[];

out gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0) out float outLength;

void GenerateLayer(vec4 pos0, vec4 pos1, vec4 pos2, int layerIndex)
{
    gl_Layer    = layerIndex;
    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix[layerIndex] * pos0;
    outLength   = length(uboMVP.position.xyz - pos0.xyz);
    EmitVertex();

    gl_Layer    = layerIndex;
    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix[layerIndex] * pos1;
    outLength   = length(uboMVP.position.xyz - pos1.xyz);
    EmitVertex();

    gl_Layer    = layerIndex;
    gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix[layerIndex] * pos2;
    outLength   = length(uboMVP.position.xyz - pos2.xyz);
    EmitVertex();
    
	EndPrimitive();
}

void main(void)
{	
    vec4 pos0 = uboMVP.modelMatrix * gl_in[0].gl_Position;
    vec4 pos1 = uboMVP.modelMatrix * gl_in[1].gl_Position;
    vec4 pos2 = uboMVP.modelMatrix * gl_in[2].gl_Position;
    
    GenerateLayer(pos0, pos1, pos2, 0);
    GenerateLayer(pos0, pos1, pos2, 1);
    GenerateLayer(pos0, pos1, pos2, 2);
    GenerateLayer(pos0, pos1, pos2, 3);
    GenerateLayer(pos0, pos1, pos2, 4);
    GenerateLayer(pos0, pos1, pos2, 5);
}