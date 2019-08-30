#version 450

layout (binding = 0) uniform sampler2D diffuseMap;
layout (binding = 1) uniform sampler2D gradientMap;

layout (location = 0) in float inGradient;

layout (location = 0) out vec4 outFragColor;

void main ()
{
	vec4 diffuse  = texture(diffuseMap, gl_PointCoord);
    vec4 gradient = texture(gradientMap, vec2(inGradient, 0));
    diffuse.xyzw *= gradient.xyzw;
	outFragColor  = diffuse;
}