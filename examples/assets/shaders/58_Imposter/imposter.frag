#version 450

layout (location = 0) in vec2 inUV0;

layout (binding = 2) uniform sampler2D originTexture;
layout (binding = 3) uniform sampler2D originNormal;

layout (location = 0) out vec4 outFragColor;

layout (binding = 4) uniform UVScaleBlock
{
	vec4 param;
} uboLight;

void main() 
{
    vec4 diffuse = texture(originTexture, inUV0);
    vec4 normal  = texture(originNormal,  inUV0);
    normal.xyz   = normal.xyz * 2.0 - 1.0; // [0, 1] -> [-1, 1]

    outFragColor.xyz = diffuse.xyz * max(dot(normal.xyz, uboLight.param.xyz), 0.3) * 2.0;
    outFragColor.w = dot(diffuse.xyz, vec3(0.2125, 0.7154, 0.0721));
    outFragColor.w = outFragColor.w > 0 ? 1.0 : 0.0;
}