#version 450

layout (location = 0) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 diffuse = vec4(1.0, 1.0, 1.0, 1.0);
    diffuse.xyz  = dot(normalize(vec3(-1, 1, -1)), inNormal) * diffuse.xyz; 
    outFragColor = diffuse;
}