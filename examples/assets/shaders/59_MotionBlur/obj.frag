#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;

layout (location = 2) in vec2 inVelocity;

layout (binding  = 2) uniform sampler2D diffuseMap;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outVelocity;

void main() 
{
    vec4 diffuse = texture(diffuseMap, inUV);
    outFragColor = diffuse;

    if (diffuse.a < 0.5) {
        discard;
    }

    outVelocity = vec4(inVelocity, 1.0, 1.0);
}