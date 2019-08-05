#version 450

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;

layout (binding  = 1) uniform sampler2D diffuseMap;

layout (location = 0) out vec4 outFragColor;
layout (location = 1) out vec4 outNormal;

void main() 
{
    vec4 diffuse  = texture(diffuseMap, inUV);
    vec3 lightDir = vec3(0, 1, -1);
    diffuse.xyz   = dot(lightDir, inNormal) * diffuse.xyz; 
    outFragColor  = diffuse;

    // [-1, 1] -> [0, 2] -> [0, 1]
    vec3 normal = (inNormal + 1) / 2;
    outNormal   = vec4(normal, 1.0);
}