#version 450

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    outFragColor.xyz = vec3(1.0, 1.0, 1.0);
    outFragColor.w   = 1.0;
}