#version 450

layout (location = 0) in vec2 inUV0;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in float inLayer;

layout (binding = 1) uniform sampler2DArray diffuseMap;

layout (location = 0) out vec4 outFragColor;

void main() 
{
    vec4 diffuse = texture(diffuseMap, vec3(inUV0, inLayer));
    if (diffuse.a < 0.5) {
        discard;
    }

    float NDotL = clamp(dot(inNormal, vec3(0, 1, 0)), 0, 1.0);
    outFragColor = diffuse * (NDotL + 0.25);
}