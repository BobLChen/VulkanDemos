#version 450

layout (location = 0) in vec3  inPosition;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	gl_Position = uboMVP.projectionMatrix * uboMVP.viewMatrix * uboMVP.modelMatrix * vec4(inPosition, 1.0);
}