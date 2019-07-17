#version 450

layout (location = 0) in vec3 inNormal;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 normal = normalize(inNormal);
    vec3 lightDir = vec3(0, 0, -1);
    float diffuse = dot(normal, lightDir);
    outFragColor = vec4(diffuse, diffuse, diffuse, 1.0);
}