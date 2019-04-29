#version 450

layout (location = 0) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec3 normal = normalize(inColor);
    outFragColor = vec4(normal.x, normal.y, normal.z, 1.0);
}
