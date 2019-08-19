#version 450

layout (location = 0) in vec2 inUV0;

layout (binding  = 1) uniform samplerCube depthTexture;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    float depth = texture(depthTexture, vec3(inUV0, 0)).r;
    // depth = (depth - 0.8) / (1.0 - 0.8);
    outFragColor = vec4(depth, depth, depth, 1.0);
}