#version 450

layout (location = 0) in vec3 inPosition;

layout (binding = 0) uniform MVPBlock 
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} uboMVP;

out gl_PerVertex 
{
    vec4  gl_Position;
	float gl_PointSize;
};

void main() 
{
	gl_PointSize = 1.0;
	gl_Position  = vec4(inPosition, 1.0);
}