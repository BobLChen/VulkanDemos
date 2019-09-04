#version 450

layout (location = 0) in float inLength;

layout (location = 0) out float outFragColor;

void main() 
{
    outFragColor = inLength;
}