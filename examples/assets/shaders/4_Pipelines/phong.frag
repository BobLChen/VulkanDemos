#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inViewDir;
layout (location = 2) in vec3 inLightDir;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec3 ambient = vec3(0.2, 0.2, 0.2);
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightDir);
	vec3 V = normalize(inViewDir);
	vec3 R = reflect(L, N);
	vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0, 1.0, 1.0);
	vec3 specular = pow(max(dot(R, V), 0.0), 32.0) * vec3(1.0, 0.0, 0.0);
	outFragColor = vec4(ambient + diffuse + specular, 1.0);		
}